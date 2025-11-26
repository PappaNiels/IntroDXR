#include "pch.hpp"
#include "DescriptorHeap.hpp"
#include "Device.hpp"

#include <Utils/Assert.hpp>

void DescriptorHeap::Initialize(HeapType type)
{
	auto device = Device::GetDevice().GetInternalDevice();
	
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = type == HeapType::RTV ? D3D12_DESCRIPTOR_HEAP_TYPE_RTV : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = type == HeapType::RTV ? D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 32;

	device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_Heap));

	m_IncreaseSize = device->GetDescriptorHandleIncrementSize(type == HeapType::RTV ? D3D12_DESCRIPTOR_HEAP_TYPE_RTV : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t DescriptorHeap::GetNextIndex()
{
	ASSERT(m_CurrentIndex + 1 < m_MaxIndex, "Heap is too small");

	return ++m_CurrentIndex;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandle(uint32_t index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Heap->GetCPUDescriptorHandleForHeapStart(), static_cast<int32_t>(index), m_IncreaseSize);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandle(uint32_t index)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_Heap->GetGPUDescriptorHandleForHeapStart(), static_cast<int32_t>(index), m_IncreaseSize);
}
