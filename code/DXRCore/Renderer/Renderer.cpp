#include "pch.hpp"
#include "Renderer.hpp"

#include "Attributes/Device.hpp"
#include "Attributes/DescriptorHeap.hpp"
#include "Attributes/SwapChain.hpp"

#include "Attributes/RaytracingPipeline.hpp"

#include "Helper.hpp" 

#include <Utils/CLI.hpp>
#include <Utils/Error.hpp>
#include <Utils/Assert.hpp>

#include <DirectXMath.h>

Renderer* g_Renderer = nullptr;

using namespace DirectX;

DescriptorHeap* Renderer::GetShaderHeap()
{
	return g_Renderer->m_ShaderHeap;
}

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

	m_SwapChain = new SwapChain();
	m_SwapChain->Initialize(m_HWND, m_Width, m_Height);

	m_RTVHeap = new DescriptorHeap();
	m_RTVHeap->Initialize(HeapType::RTV);

	m_ShaderHeap = new DescriptorHeap();
	m_ShaderHeap->Initialize(HeapType::Shader);
	
	m_RTV = m_RTVHeap->GetNextIndex();
	m_UAV = m_ShaderHeap->GetNextIndex();

	CreateRenderTarget();
	CreateCommandLists();

	g_Renderer = this;

	InitializeSample();
}

void Renderer::Shutdown()
{
	m_Device->GetCommandQueue().Flush();
}

void Renderer::Render()
{
	auto& commandQueue = m_Device->GetCommandQueue();
	auto& cmdList = m_CommandList[m_FrameNumber % 2];

	if (m_FrameNumber > 1)
	{
		//commandQueue.WaitForFence(m_FenceValue[m_FrameNumber % 2]);

		m_CommandListAllocator[m_FrameNumber % 2]->Reset();
		cmdList->Reset(m_CommandListAllocator[m_FrameNumber % 2].Get(), nullptr);
	}

	float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

	cmdList->ClearRenderTargetView(m_RTVHeap->GetCPUHandle(m_RTV), color, 0, nullptr);

	cmdList->SetDescriptorHeaps(1, m_ShaderHeap->GetHeap().GetAddressOf());

	RenderSample(cmdList);

	commandQueue.ExecuteCommandLists({ cmdList.Get() });

	m_SwapChain->Present(m_RenderTarget);

	m_FrameNumber++;
}

void Renderer::Resize(uint32_t width, uint32_t height)
{
	if (width == m_Width && height == m_Height)
	{
		return;
	}

	m_Width = std::max(width, 1u);
	m_Height = std::max(height, 1u);

	m_Device->Flush();

	CreateRenderTarget();
	m_SwapChain->Resize(m_Width, m_Height);
}

void Renderer::CreateRenderWindow()
{
	std::string windowName = "Intro to DirectX Raytracing | " + ms_SampleName;

	if (GetCLI().Validation)
	{
		windowName += " [Validation]";
	}

	if (GetCLI().Warp)
	{
		windowName += " [WARP]";
	}

	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(wndClass);
	wndClass.hInstance = GetModuleHandleW(nullptr);
	wndClass.lpszClassName = "RenderWindowClass";
	wndClass.style = 0;
	wndClass.lpfnWndProc = &Renderer::WndProc;
	wndClass.hCursor = nullptr;

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
	int32_t width = static_cast<int32_t>(R.right - R.left);
	int32_t height  = static_cast<int32_t>(R.bottom - R.top);

	m_HWND = CreateWindow("RenderWindowClass", windowName.c_str(), WS_OVERLAPPEDWINDOW, (desktop.right / 2) - (width / 2), (desktop.bottom / 2) - (height / 2), width, height, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);

	if (m_HWND == nullptr)
	{
		FatalError("Failed to create a window");
		return;
	}

	ShowWindow(m_HWND, SW_SHOW);
	UpdateWindow(m_HWND);
}

void Renderer::CreateRenderTarget()
{
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, m_Width, m_Height, 1, 1);
	resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_CLEAR_VALUE clearColor = {};
	clearColor.Color[0] = 1.0f;
	clearColor.Color[3] = 1.0f;
	clearColor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	auto hr = m_Device->GetInternalDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearColor, IID_PPV_ARGS(&m_RenderTarget));

	if (FAILED(hr))
	{
		FatalError("Failed to create render target. HResult: 0x%08X", hr);
		return;
	}
	
	m_Device->GetInternalDevice()->CreateRenderTargetView(m_RenderTarget.Get(), nullptr, m_RTVHeap->GetCPUHandle(m_RTV));
	m_Device->GetInternalDevice()->CreateUnorderedAccessView(m_RenderTarget.Get(), nullptr, nullptr, m_ShaderHeap->GetCPUHandle(m_UAV));
}

void Renderer::CreateCommandLists()
{
	auto device = Device::GetDevice().GetInternalDevice();

	for (uint32_t i = 0; i < 2; i++)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandListAllocator[i]));
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandListAllocator[i].Get(), nullptr, IID_PPV_ARGS(&m_CommandList[i]));
	}
}

LRESULT Renderer::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE)
	{
		auto* createStr = reinterpret_cast<LPCREATESTRUCT*>(lParam);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStr));
		return 0;
	}

	auto* renderer = g_Renderer;

	if (renderer == nullptr)
	{
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	switch (message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		break;
	case WM_SIZE:
	{
		RECT clientRect = {};
		GetClientRect(hwnd, &clientRect);

		uint32_t width = static_cast<uint32_t>(clientRect.right - clientRect.left);
		uint32_t height = static_cast<uint32_t>(clientRect.bottom - clientRect.top);

		renderer->Resize(width, height);

		return 0;
	}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}