#pragma once

#include <queue>

class SwapChain
{
public:
	void Initialize(HWND hwnd, uint32_t width, uint32_t height);
	void Present(Microsoft::WRL::ComPtr<ID3D12Resource> renderTarget);
	void Resize(uint32_t width, uint32_t height);

	static constexpr uint32_t GetBackBufferCount()
	{
		return ms_BackBufferCount;
	}
protected:
	friend class Renderer;
	SwapChain() = default;
private:
	static constexpr uint32_t ms_BackBufferCount = 2u;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_BackBuffers[ms_BackBufferCount];

	uint64_t m_FenceValues[ms_BackBufferCount];

	struct CommandList
	{
		uint64_t FenceValue;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> CmdList;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
	};

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> m_CommandList[2];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandListAllocator[2];

	uint64_t m_FrameNumber = 0;

	uint32_t m_CurrentBackBuffer;

	uint32_t m_TearingSupported : 1;
	uint32_t m_VSyncEnabled : 1;
};

static_assert(SwapChain::GetBackBufferCount() > 1, "The back buffer count needs to be more than 1");

