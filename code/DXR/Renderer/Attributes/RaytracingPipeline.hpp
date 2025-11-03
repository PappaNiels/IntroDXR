#pragma once

#include <wrl/client.h>

struct HitGroupEntry
{
	HitGroupEntry(std::wstring_view entry, std::wstring_view hitGroup, D3D12_HIT_GROUP_TYPE type)
		: ShaderEntry(entry)
		, HitGroup(hitGroup)
		, Type(type)
	{
	}

	std::wstring_view ShaderEntry;
	std::wstring_view HitGroup;
	D3D12_HIT_GROUP_TYPE Type;
};

struct MissShaderEntry
{
	MissShaderEntry(std::wstring_view entry)
		: EntryName(entry)
	{
	}

	std::wstring_view EntryName;
};

struct RayGenEntry
{
	std::wstring_view EntryName;
};

struct RaytracingPipelineDesc
{
	RayGenEntry RayGenEntry;
	std::vector<HitGroupEntry> HitGroups;
	std::vector<MissShaderEntry> MissShaders;
};

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

	RaytracingPipeline(const RaytracingPipelineDesc& desc);
protected:
	friend class Renderer;
	RaytracingPipeline(std::string_view name, void* shaderCode);

private:
	void CreateGlobalRootSignature(const RaytracingPipelineDesc& desc);
	//void CreateLocalRootSignatures();

	void CreateShaderTables(const RaytracingPipelineDesc& desc);

	void CreatePipeline(const RaytracingPipelineDesc& desc);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12StateObject> m_Pipeline;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_RayGenShaderTable;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_HitShaderTable;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_MissShaderTable;
};

