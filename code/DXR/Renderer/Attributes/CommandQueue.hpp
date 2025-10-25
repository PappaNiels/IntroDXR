#pragma once

class CommandQueue
{
public:
	void Initialize();

	uint64_t ExecuteCommandLists(const std::vector<ID3D12GraphicsCommandList7*> cmdList);
	void WaitForFence(uint64_t fence);

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const
	{
		return m_CommandQueue;
	}

	void Flush();
protected:
	friend class Device;
	CommandQueue() = default;
private:
	uint64_t Signal();
	bool IsFenceCompleted(uint64_t fence);

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;

	uint64_t m_FenceValue = 1;
};

