#pragma once

#include <wrl/client.h>
#include <dxgi1_6.h>

#define SAMPLE(x) Renderer* CreateSample() { return new x();} std::wstring Renderer::ms_SampleName = L#x;

class Renderer
{
public:
	static class DescriptorHeap* GetShaderHeap();

	Renderer();
	Renderer(uint32_t width, uint32_t height);
	~Renderer();

	void Initialize();
	void Shutdown();

	void Render();
	virtual void Update([[maybe_unused]] float deltaTime) {};

	virtual void InitializeSample() {};
	virtual void RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7>) {};

	void Resize(uint32_t width, uint32_t height);

private:
	void CreateRenderWindow();
	void CreateRenderTarget();
	void CreateCommandLists();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	class Device* m_Device;
	class SwapChain* m_SwapChain;

	HWND m_HWND;

	class DescriptorHeap* m_RTVHeap = nullptr;
	class DescriptorHeap* m_ShaderHeap = nullptr;

	class RaytracingPipeline* m_Pipeline = nullptr;

	ComPtr<ID3D12Resource> m_RenderTarget;
	uint32_t m_RTV;
	uint32_t m_UAV;

	ComPtr<ID3D12GraphicsCommandList7> m_CommandList[2];
	ComPtr<ID3D12CommandAllocator> m_CommandListAllocator[2];

	uint64_t m_FrameNumber = 0;

	uint64_t m_FenceValue[2] = {};

	uint32_t m_Width;
	uint32_t m_Height;

	static std::wstring ms_SampleName;
};

