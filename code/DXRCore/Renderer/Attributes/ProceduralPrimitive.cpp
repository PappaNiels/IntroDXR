#include "pch.hpp"
#include "ProceduralPrimitive.hpp"

#include "Device.hpp"

#include <Renderer/Helper.hpp>

void ProceduralPrimitive::AddEntry(const Entry& entry)
{
	m_Entries.push_back(entry);
}

void ProceduralPrimitive::BuildBLAS()
{
	ASSERT(!m_Entries.empty(), "No procedural primitive entries in the BLAS");

	auto device = Device::GetDevice().GetInternalDevice();

	AllocateUploadBuffer(device.Get(), m_Entries.data(), sizeof(Entry), &m_AABBs);

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
	geometryDesc.AABBs.AABBs = { m_AABBs->GetGPUVirtualAddress(), sizeof(Entry) };
	geometryDesc.AABBs.AABBCount = m_Entries.size();
	geometryDesc.Flags = m_Flags;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blasInput = {};
	blasInput.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	blasInput.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	blasInput.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	blasInput.NumDescs = 1;
	blasInput.pGeometryDescs = &geometryDesc;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&blasInput, &info);

	ASSERT(info.ResultDataMaxSizeInBytes > 0, "Failed to get prebuild data");

	Microsoft::WRL::ComPtr<ID3D12Resource> scratchResource;
	AllocateUAVBuffer(device.Get(), info.ScratchDataSizeInBytes, &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
	AllocateUAVBuffer(device.Get(), info.ResultDataMaxSizeInBytes, &m_BLAS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"BottomLevelAccelerationStructure");

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	bottomLevelBuildDesc.Inputs = blasInput;
	bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
	bottomLevelBuildDesc.DestAccelerationStructureData = m_BLAS->GetGPUVirtualAddress();

	auto& cmdQueue = Device::GetDevice().GetCommandQueue();
	auto cmdList = CreateCommandList();

	cmdList.CommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);

	auto fence = cmdQueue.ExecuteCommandLists({ cmdList.CommandList.Get() });
	cmdQueue.WaitForFence(fence);
}