#include "pch.hpp"
#include "SwapChain.hpp"

#include "Device.hpp"
#include "CommandQueue.hpp"

#include <Utils/CLI.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Error.hpp>

void SwapChain::Initialize(HWND hwnd, uint32_t width, uint32_t height)
{
    auto& device = Device::GetDevice();

    Microsoft::WRL::ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory4;

    uint32_t flags = 0;

    if (GetCLI().Validation)
    {
        flags |= DXGI_CREATE_FACTORY_DEBUG;
    }

    ASSERT(SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&dxgiFactory4))), "Failed to create a dxgi factory");

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = ms_BackBufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = m_TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ID3D12CommandQueue* pCommandQueue = device.GetCommandQueue().GetCommandQueue().Get();

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;

    auto hr = dxgiFactory4->CreateSwapChainForHwnd(
        pCommandQueue,
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1);

    if (FAILED(hr))
    {
        FatalError("Failed to create DXGISwapChain");
    }

    ASSERT(SUCCEEDED(dxgiFactory4->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)), "Failed to associate window with swapchain");
    ASSERT(SUCCEEDED(swapChain1.As(&dxgiSwapChain4)), "");

    m_CurrentBackBuffer = dxgiSwapChain4->GetCurrentBackBufferIndex();
    m_SwapChain = dxgiSwapChain4;
    
    for (uint32_t i = 0; i < ms_BackBufferCount; i++)
    {
        m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i]));
    }

    for (uint32_t i = 0; i < 2; i++)
    {
        device.GetInternalDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandListAllocator[i]));
        device.GetInternalDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandListAllocator[i].Get(), nullptr, IID_PPV_ARGS(&m_CommandList[i]));
    }
}

void SwapChain::Present(Microsoft::WRL::ComPtr<ID3D12Resource> renderTarget)
{
	auto device = Device::GetDevice().GetInternalDevice();
	auto& commandQueue = Device::GetDevice().GetCommandQueue();
    auto& cmdList = m_CommandList[m_FrameNumber % 2];

    if (m_FrameNumber > 1)
    {
        m_CommandListAllocator[m_FrameNumber % 2]->Reset();
        cmdList->Reset(m_CommandListAllocator[m_FrameNumber % 2].Get(), nullptr);
    }

	CD3DX12_RESOURCE_BARRIER barriers[2];
	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_BackBuffers[m_CurrentBackBuffer].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdList->ResourceBarrier(2, barriers);
	cmdList->CopyResource(m_BackBuffers[m_CurrentBackBuffer].Get(), renderTarget.Get());

	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_BackBuffers[m_CurrentBackBuffer].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);

	cmdList->ResourceBarrier(2, barriers);

	m_FenceValues[m_CurrentBackBuffer] = commandQueue.ExecuteCommandLists({cmdList.Get()});

	m_SwapChain->Present(m_VSyncEnabled ? 1 : 0, m_TearingSupported ? DXGI_PRESENT_ALLOW_TEARING : 0);
	m_CurrentBackBuffer = m_SwapChain->GetCurrentBackBufferIndex();

	commandQueue.WaitForFence(m_FenceValues[m_CurrentBackBuffer]);

    m_FrameNumber++;
}

void SwapChain::Resize(uint32_t width, uint32_t height)
{
	m_SwapChain->ResizeBuffers(ms_BackBufferCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	for (uint32_t i = 0; i < ms_BackBufferCount; i++)
	{
		m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i]));
	}
}