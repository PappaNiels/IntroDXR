#include "pch.hpp"
#include "Device.hpp"

#include "CommandQueue.hpp"

#include <Utils/Error.hpp>
#include <Utils/CLI.hpp>
#include <Utils/Assert.hpp>

Device* g_Device;

Device& Device::GetDevice()
{
	ASSERT(g_Device != nullptr, "The d3d12 device was not initialized");
	return *g_Device;
}

void Device::Initialize()
{
	CreateAdapter();
	CreateDevice();

	g_Device = this;

	m_CommandQueue = std::unique_ptr<CommandQueue>(new CommandQueue());
	m_CommandQueue->Initialize();
}

void Device::CreateAdapter()
{
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter4;
	uint32_t flags = 0;

	if (GetCLI().Validation)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug1> debugInterface;
		ASSERT(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))), "Failed to get the debug interface");
		debugInterface->EnableDebugLayer();
		debugInterface->SetEnableGPUBasedValidation(FALSE);

		flags |= DXGI_CREATE_FACTORY_DEBUG;
		m_HasDebugLayersEnabled = true;
	}

	ASSERT(SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory))), "Failed to create a DXGI factory");

	if (GetCLI().Warp)
	{
		ASSERT(SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(&m_Adapter))), "");
	}
	else
	{
		uint64_t maxDedicatedMemory = 0;

		for (uint32_t i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_ADAPTER_DESC1 adapterDesc = {};
			adapter->GetDesc1(&adapterDesc);

			if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) && adapterDesc.DedicatedVideoMemory > maxDedicatedMemory)
			{
				maxDedicatedMemory = adapterDesc.DedicatedVideoMemory;
				ASSERT(SUCCEEDED(adapter.As(&adapter4)), "Failed to convert the adapter to an adapter4");
			}
		}

		m_Adapter = adapter4;
	}
}

void Device::CreateDevice()
{
	ASSERT(SUCCEEDED(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device))), "Failed to create the device");

	CheckFeatureSupport();

	if (!m_HasDebugLayersEnabled)
	{
		return;
	}

	Microsoft::WRL::ComPtr<ID3D12InfoQueue1> pInfoQueue;
	if (SUCCEEDED(m_Device.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		D3D12_MESSAGE_SEVERITY Severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		ASSERT(SUCCEEDED(pInfoQueue->PushStorageFilter(&NewFilter)), "Failed to push filters");

		DWORD cookie = 0;
		ASSERT(SUCCEEDED(pInfoQueue->RegisterMessageCallback(&Device::Debug, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &cookie)), "Failed to register message callback");
	}
}

void Device::CheckFeatureSupport()
{
	{
		D3D12_FEATURE_DATA_SHADER_MODEL feature = { D3D_SHADER_MODEL_6_6 };

		if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &feature, sizeof(feature))) || feature.HighestShaderModel < D3D_SHADER_MODEL_6_6)
		{
			FatalError("This graphics card does not support shader model 6.6. This application relies on shader model 6.6 for bindless rendering. Please update the graphics card drivers and try again...");
		}
	}

	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 feature = {};

		if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &feature, sizeof(feature))))
		{
			DebugBreak();
		}

		if (feature.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
		{
			FatalError("This graphics card does not support DirectX Raytracing. Exiting....");
		}
	}
}

void Device::Debug(D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext)
{
	std::string str = "[D3D12] ";

	switch (Severity)
	{
	case D3D12_MESSAGE_SEVERITY_CORRUPTION:
		str += "[CORRUPTION] ";
		break;
	case D3D12_MESSAGE_SEVERITY_ERROR:
		str += "[ERROR] ";
		break;
	case D3D12_MESSAGE_SEVERITY_WARNING:
		str += "[WARNING] ";
		break;
	case D3D12_MESSAGE_SEVERITY_INFO:
		str += "[INFO] ";
		break;
	case D3D12_MESSAGE_SEVERITY_MESSAGE:
		str += "[MESSAGE] ";
		break;
	default:
		break;
	}

	str += pDescription;
	str += '\n';

	printf(str.c_str());
}
