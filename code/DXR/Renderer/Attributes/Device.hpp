#pragma once

#include <wrl/client.h>

class Device
{
public:
	Device() = default;

	void Initialize();
private:
	void CreateAdapter();
	void CreateDevice();

	void CheckFeatureSupport();

	static void Debug(D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext);

	Microsoft::WRL::ComPtr<IDXGIAdapter> m_Adapter;
	Microsoft::WRL::ComPtr<ID3D12Device5> m_Device;

	bool m_HasDebugLayersEnabled = false;
};

