#pragma once

#include <wrl/client.h>

#include <Renderer/Attributes/CommandQueue.hpp>

class Device
{
public:
	static Device& GetDevice();

	void Initialize();

	Microsoft::WRL::ComPtr<ID3D12Device5> GetInternalDevice() const
	{
		return m_Device;
	}

	CommandQueue& GetCommandQueue()
	{
		return *m_CommandQueue;
	}

protected:
	friend class Renderer;
	Device() = default;
private:

	void CreateAdapter();
	void CreateDevice();

	void CheckFeatureSupport();

	static void Debug(D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext);

	Microsoft::WRL::ComPtr<IDXGIAdapter> m_Adapter;
	Microsoft::WRL::ComPtr<ID3D12Device5> m_Device;

	std::unique_ptr<class CommandQueue> m_CommandQueue;

	bool m_HasDebugLayersEnabled = false;
};

