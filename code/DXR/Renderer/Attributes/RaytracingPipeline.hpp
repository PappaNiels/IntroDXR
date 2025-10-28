#pragma once

#include <wrl/client.h>

class RaytracingPipeline
{
public:

	enum
	{
		Constants, // 17
		TLAS,
		Count
	};

	Microsoft::WRL::ComPtr<ID3D12StateObject> GetStateObject() const
	{
		return m_Pipeline;
	}

	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const
	{
		return m_RootSignature;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> GetHitGroupTable() const
	{
		return m_HitShaderTable;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> GetMissTable() const
	{
		return m_MissShaderTable;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> GetRayGenTable() const
	{
		return m_RayGenShaderTable;
	}

protected:
	friend class Renderer;
	RaytracingPipeline(std::string_view name, void* shaderCode);

private:
	void CreateGlobalRootSignature();
	//void CreateLocalRootSignatures();

	void CreateShaderTables();

	void CreatePipeline();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12StateObject> m_Pipeline;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_RayGenShaderTable;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_HitShaderTable;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_MissShaderTable;
};

