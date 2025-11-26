#include "pch.hpp"
#include "CommandQueue.hpp"

#include "Device.hpp"

#include <Utils/Error.hpp>

#include <limits>

void CommandQueue::Initialize()
{
	auto device = Device::GetDevice().GetInternalDevice();

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	auto hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue));

	if (FAILED(hr))
	{
		FatalError("Failed to create a command queue");
	}

	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));

	if (FAILED(hr))
	{
		FatalError("Failed to create a fence");
	}
}

uint64_t CommandQueue::ExecuteCommandLists(const std::vector<ID3D12GraphicsCommandList7*> cmdLists)
{
	for (auto cmdList : cmdLists)
	{
		cmdList->Close();
	}

	ID3D12CommandList* const* cmdListsRaw = reinterpret_cast<ID3D12CommandList* const*>(cmdLists.data());

	m_CommandQueue->ExecuteCommandLists(static_cast<uint32_t>(cmdLists.size()), cmdListsRaw);

	auto fence = Signal();

	return fence;
}

void CommandQueue::WaitForFence(uint64_t fence)
{
	if (!IsFenceCompleted(fence))
	{
		auto event = ::CreateEvent(0, FALSE, FALSE, nullptr);

		if (event)
		{
			m_Fence->SetEventOnCompletion(fence, event);
			WaitForSingleObjectEx(event, std::numeric_limits<uint32_t>::max(), FALSE);

			CloseHandle(event);
		}
	}
}

uint64_t CommandQueue::Signal()
{
	auto fence = ++m_FenceValue;
	m_CommandQueue->Signal(m_Fence.Get(), fence);
	return fence;
}

bool CommandQueue::IsFenceCompleted(uint64_t fence)
{
	return m_Fence->GetCompletedValue() >= fence;
}

void CommandQueue::Flush()
{
	auto fence = Signal();
	WaitForFence(fence);
}