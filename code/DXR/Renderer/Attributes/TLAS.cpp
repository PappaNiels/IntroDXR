#include "pch.hpp"
#include "TLAS.hpp"

#include "Device.hpp"
#include "Mesh.hpp"
#include <Renderer/Helper.hpp>

void TLAS::AddMesh(MeshInstance* mesh)
{
	mesh->IsDirty = true; // Ensure that it is labeled as dirty, so we rebuild the tlas

	m_Meshes.push_back(mesh);
}

void TLAS::Build()
{
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInput = {};
	tlasInput.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	tlasInput.Flags = buildFlags;
	tlasInput.NumDescs = static_cast<uint32_t>(m_Meshes.size());
	tlasInput.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	auto device = Device::GetDevice().GetInternalDevice();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&tlasInput, &info);
	ASSERT(info.ResultDataMaxSizeInBytes > 0, "...");

	Microsoft::WRL::ComPtr<ID3D12Resource> scratchResource;
	AllocateUAVBuffer(device.Get(), info.ScratchDataSizeInBytes, &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
	AllocateUAVBuffer(device.Get(), info.ResultDataMaxSizeInBytes, &m_TLAS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TopLevelAccelerationStructure");

	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> meshes;
	meshes.reserve(m_Meshes.size());

	for (const auto& mesh : m_Meshes)
	{
		if (mesh == nullptr || !mesh)
		{
			continue;
		}

		auto transform = mesh->GetMatrix();
		auto m = XMMatrixTranspose(transform);

		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		instanceDesc.InstanceMask = 1;
		instanceDesc.AccelerationStructure = mesh->GetBLASAddress();
		instanceDesc.InstanceID = 0;
		
		/*for (int i = 0; i < 3; ++i)
		{
			instanceDesc.Transform[i][0] = DirectX::XMVectorGetX(m.r[i]);
			instanceDesc.Transform[i][1] = DirectX::XMVectorGetY(m.r[i]);
			instanceDesc.Transform[i][2] = DirectX::XMVectorGetZ(m.r[i]);
			instanceDesc.Transform[i][3] = DirectX::XMVectorGetW(m.r[i]);
		}*/

		instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;

		meshes.push_back(instanceDesc);
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> instanceDesc;
	AllocateUploadBuffer(device.Get(), meshes.data(), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * meshes.size(), &instanceDesc, L"InstanceDescs");

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	tlasInput.InstanceDescs = instanceDesc->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs = tlasInput;
	topLevelBuildDesc.DestAccelerationStructureData = m_TLAS->GetGPUVirtualAddress();
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();

	auto& cmdQueue = Device::GetDevice().GetCommandQueue();
	auto cmdList = CreateCommandList();

	cmdList.CommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	// consider making batches rather calling the commandqueue once per mesh
	auto fence = cmdQueue.ExecuteCommandLists({ cmdList.CommandList.Get() });
	cmdQueue.WaitForFence(fence);
}

void TLAS::Build(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList)
{
	bool isDirty = false;
	for (const auto& mesh : m_Meshes)
	{
		if (mesh == nullptr || !mesh)
		{
			continue;
		}

		if (mesh->IsDirty)
		{
			// We found a mesh that needs to be updated. Rebuild/refit is needed
			isDirty = true;
			break;
		}
	}

	// Check if the TLAS is dirty
	if (!isDirty)
	{
		return;
	}

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInput = {};
	tlasInput.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	tlasInput.Flags = buildFlags;
	tlasInput.NumDescs = static_cast<uint32_t>(m_Meshes.size());
	tlasInput.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	auto device = Device::GetDevice().GetInternalDevice();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&tlasInput, &info);
	ASSERT(info.ResultDataMaxSizeInBytes > 0, "...");

	Microsoft::WRL::ComPtr<ID3D12Resource> scratchResource;
	AllocateUAVBuffer(device.Get(), info.ScratchDataSizeInBytes, &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
	AllocateUAVBuffer(device.Get(), info.ResultDataMaxSizeInBytes, &m_TLAS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TopLevelAccelerationStructure");

	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> meshes;
	meshes.reserve(m_Meshes.size());

	for (const auto& mesh : m_Meshes)
	{
		if (mesh == nullptr || !mesh)
		{
			continue;
		}

		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
		instanceDesc.InstanceMask = 1;
		instanceDesc.AccelerationStructure = mesh->GetBLASAddress();
		instanceDesc.InstanceID = 0;

		meshes.push_back(instanceDesc);
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> instanceDesc;
	AllocateUploadBuffer(device.Get(), meshes.data(), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * meshes.size(), &instanceDesc, L"InstanceDescs");

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	tlasInput.InstanceDescs = instanceDesc->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs = tlasInput;
	topLevelBuildDesc.DestAccelerationStructureData = m_TLAS->GetGPUVirtualAddress();
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();

	cmdList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_TLAS.Get());
	cmdList->ResourceBarrier(1, &barrier);
}
