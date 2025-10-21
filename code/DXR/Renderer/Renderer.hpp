#pragma once

#include <wrl/client.h>
#include <dxgi1_6.h>

class Renderer
{
public:
	Renderer();
	Renderer(uint32_t width, uint32_t height);
	~Renderer();

	void Initialize();
	void Shutdown();

	void Render();
	void Resize(uint32_t width, uint32_t height);

private:
	void CreateRenderWindow();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	class Device* m_Device;

	ComPtr<IDXGISwapChain> m_SwapChain;

	HWND m_HWND;

	uint32_t m_Width;
	uint32_t m_Height;
};

