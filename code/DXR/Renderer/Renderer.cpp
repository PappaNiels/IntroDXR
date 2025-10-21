#include "pch.hpp"
#include "Renderer.hpp"

#include "Attributes/Device.hpp"

#include <Utils/CLI.hpp>
#include <Utils/Error.hpp>

Renderer::Renderer()
	: m_Device(nullptr)
	, m_HWND(nullptr)
	, m_Width(1280)
	, m_Height(720)
{
}

Renderer::Renderer(uint32_t width, uint32_t height)
	: m_Device(nullptr)
	, m_HWND(nullptr)
	, m_Width(width)
	, m_Height(height)
{
}

Renderer::~Renderer()
{
}

void Renderer::Initialize()
{
	CreateRenderWindow();

	m_Device = new Device();
	m_Device->Initialize();
}

void Renderer::Shutdown()
{
}

void Renderer::Render()
{
}

void Renderer::Resize(uint32_t width, uint32_t height)
{
}

void Renderer::CreateRenderWindow()
{
	std::wstring windowName = L"Intro to DirectX Raytracing";

	if (GetCLI().Validation)
	{
		windowName += L" [Validation]";
	}

	if (GetCLI().Warp)
	{
		windowName += L" [WARP]";
	}

	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(wndClass);
	wndClass.hInstance = GetModuleHandleW(nullptr);
	wndClass.lpszClassName = L"RenderWindowClass";
	wndClass.style = 0;
	wndClass.lpfnWndProc = &Renderer::WndProc;

	auto hr = RegisterClassEx(&wndClass);
	if (FAILED(hr))
	{
		FatalError("Failed to register window class. HResult: %x", hr);
	}

	RECT desktop;
	const HWND desktopHandle = GetDesktopWindow();
	GetWindowRect(desktopHandle, &desktop);

	RECT R = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_HWND = CreateWindow(L"RenderWindowClass", windowName.c_str(), WS_OVERLAPPEDWINDOW, (desktop.right / 2) - (m_Width / 2), (desktop.bottom / 2) - (m_Height / 2), m_Width, m_Height, nullptr, nullptr, GetModuleHandleW(nullptr), this);

	if (m_HWND == nullptr)
	{
		FatalError("Failed to create a window");
	}

	ShowWindow(m_HWND, SW_SHOW);
	UpdateWindow(m_HWND);
}

LRESULT Renderer::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE)
	{
		auto* createStr = reinterpret_cast<LPCREATESTRUCT*>(lParam);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStr));
		return 0;
	}

	auto* renderer = reinterpret_cast<Renderer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));


	return DefWindowProc(hwnd, message, wParam, lParam);
}
