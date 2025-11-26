#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <unordered_set>

class TLAS
{
public:
	TLAS() = default;

	void AddMesh(class MeshInstance* mesh);
	void Build();
	void Build(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList);

	D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress() const
	{
		return m_TLAS->GetGPUVirtualAddress();
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGeometryData() const
	{
		return m_GeometryData->GetGPUVirtualAddress();
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_TLAS;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_GeometryData;

	std::vector<class MeshInstance*> m_Meshes;
};

