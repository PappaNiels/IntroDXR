#include "pch.hpp"
#include "Mesh.hpp"

#include "Device.hpp"
#include "DescriptorHeap.hpp"

#include <DXRCore/Renderer/Helper.hpp>
#include <DXRCore/Renderer/Renderer.hpp>

void Mesh::BuildBLAS()
{
	ASSERT(m_IndexSize == sizeof(uint32_t) || m_IndexSize == sizeof(uint16_t), "Incorrect index size specified.");
	ASSERT(m_IndexBuffer != nullptr, "Index buffer was not present");
	ASSERT(m_PositionBuffer != nullptr, "Position buffer was not present");

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = m_IndexBuffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = static_cast<UINT>(m_IndexBuffer->GetDesc().Width) / m_IndexSize;
	geometryDesc.Triangles.IndexFormat = m_IndexSize == sizeof(uint32_t) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = static_cast<UINT>(m_PositionBuffer->GetDesc().Width) / (sizeof(float) * 3); // float3
	geometryDesc.Triangles.VertexBuffer.StartAddress = m_PositionBuffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(float) * 3;
	geometryDesc.Flags = m_Flags;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blasInput = {};
	blasInput.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	blasInput.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	blasInput.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	blasInput.NumDescs = 1;
	blasInput.pGeometryDescs = &geometryDesc;

	auto device = Device::GetDevice().GetInternalDevice();

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

	// consider making batches rather calling the commandqueue once per mesh
	auto fence = cmdQueue.ExecuteCommandLists({ cmdList.CommandList.Get()});
	cmdQueue.WaitForFence(fence);
}

void Mesh::SetBufferData(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, uint64_t numComponents, uint64_t componentSize, const void* data)
{
	auto device = Device::GetDevice().GetInternalDevice();

	AllocateUploadBuffer(device.Get(), data, numComponents * componentSize, &buffer);
}

void Mesh::CreateSRV(Microsoft::WRL::ComPtr<ID3D12Resource> res, uint32_t size, uint32_t numComponents, uint32_t& srv)
{
	if (srv != static_cast<uint32_t>(-1))
	{
		return;
	}

	ASSERT(m_VertexCount > 0, "Vertex count was zero");

	auto* shaderHeap = Renderer::GetShaderHeap();
	auto device = Device::GetDevice().GetInternalDevice();
	srv = shaderHeap->GetNextIndex();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Buffer.StructureByteStride = size;
	desc.Buffer.NumElements = numComponents;

	device->CreateShaderResourceView(res.Get(), &desc, shaderHeap->GetCPUHandle(srv));
}
