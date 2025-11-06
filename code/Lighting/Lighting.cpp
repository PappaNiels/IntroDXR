#include "pch.hpp"

#include <DXR/Renderer/Renderer.hpp>

#include <DXR/Renderer/Attributes/TLAS.hpp>
#include <DXR/Renderer/Attributes/Mesh.hpp>
#include <DXR/Renderer/Attributes/RaytracingPipeline.hpp>

#include <DirectXMath.h>

#if defined _DEBUG
#include "Shaders/RaytracingLighting_Debug.hpp"
#else 
#include "Shaders/RaytracingLighting_Release.hpp"
#endif

using namespace DirectX;

class Lighting : public Renderer
{
	enum
	{
		RenderTarget,
		BVH,
		Count
	};

	void InitializeSample() override;
	void RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList) override;

	void Update(float deltaTime) override;
private:
	TLAS* m_TLAS = nullptr;
	Mesh* m_Mesh = nullptr;
	MeshInstance* m_MeshInstance = nullptr;
};

SAMPLE(Lighting)

std::wstring Renderer::ms_SampleName = L"Lighting";

void Lighting::InitializeSample()
{
	XMFLOAT3 positions[] = {
		XMFLOAT3(-0.5f, -0.5f, +0.5f),
		XMFLOAT3(-0.5f, +0.5f, +0.5f),
		XMFLOAT3(+0.5f, +0.5f, +0.5f),
		XMFLOAT3(+0.5f, -0.5f, +0.5f),

		XMFLOAT3(+0.5f, -0.5f, -0.5f),
		XMFLOAT3(+0.5f, +0.5f, -0.5f),
		XMFLOAT3(-0.5f, +0.5f, -0.5f),
		XMFLOAT3(-0.5f, -0.5f, -0.5f),

		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(-0.5f, +0.5f, -0.5f),
		XMFLOAT3(-0.5f, +0.5f, +0.5f),
		XMFLOAT3(-0.5f, -0.5f, +0.5f),

		XMFLOAT3(+0.5f, -0.5f, +0.5f),
		XMFLOAT3(+0.5f, +0.5f, +0.5f),
		XMFLOAT3(+0.5f, +0.5f, -0.5f),
		XMFLOAT3(+0.5f, -0.5f, -0.5f),

		XMFLOAT3(-0.5f, +0.5f, +0.5f),
		XMFLOAT3(-0.5f, +0.5f, -0.5f),
		XMFLOAT3(+0.5f, +0.5f, -0.5f),
		XMFLOAT3(+0.5f, +0.5f, +0.5f),

		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(-0.5f, -0.5f, +0.5f),
		XMFLOAT3(+0.5f, -0.5f, +0.5f),
		XMFLOAT3(+0.5f, -0.5f, -0.5f)
	};

	uint32_t indices[] = {
		0,  1,  2,  0,  2,  3,
		4,  5,  6,  4,  6,  7,
		8,  9, 10,  8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};

	m_Mesh = new Mesh();
	m_Mesh->SetPositionBuffer(_countof(positions), positions);
	m_Mesh->SetIndexBuffer(_countof(indices), indices);
	m_Mesh->BuildBLAS();

	m_MeshInstance = new MeshInstance();
	m_MeshInstance->Mesh = m_Mesh;

	m_TLAS = new TLAS();
	m_TLAS->AddMesh(m_MeshInstance);
	m_TLAS->Build();

	RaytracingPipelineDesc desc = {};
	desc.RayGenEntry.EntryName = L"RayGenMain";
	desc.HitGroups.emplace_back(L"ClosestMain", L"HitGroup", D3D12_HIT_GROUP_TYPE_TRIANGLES);
	desc.MissShaders.emplace_back(L"MissMain");
	desc.ShaderCode.pShaderBytecode = g_RaytracingLighting;
	desc.ShaderCode.BytecodeLength = sizeof(g_RaytracingLighting);
	desc.RecursionDepth = 1;

	desc.AttributeSize = sizeof(float) * 2;
	desc.PayloadSize = sizeof(float) * 4;

	CD3DX12_ROOT_PARAMETER params[Count] = {};
	params[RenderTarget].InitAsConstants(1, 0);
	params[BVH].InitAsShaderResourceView(0);

	desc.RootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(params), params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);

	m_Pipeline = new RaytracingPipeline(desc);
}


void Lighting::RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList)
{
	//m_TLAS->Build(cmdList);

	cmdList->SetComputeRootSignature(m_Pipeline->GetRootSignature().Get());
	cmdList->SetComputeRoot32BitConstants(0, 1, &m_UAV, 0);
	cmdList->SetComputeRootShaderResourceView(1, m_TLAS->GetVirtualAddress());

	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	dispatchDesc.HitGroupTable.StartAddress = m_Pipeline->GetHitGroupTable()->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = m_Pipeline->GetHitGroupTable()->GetDesc().Width;
	dispatchDesc.HitGroupTable.StrideInBytes = m_Pipeline->GetShaderTableSize();
	dispatchDesc.MissShaderTable.StartAddress = m_Pipeline->GetMissTable()->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = m_Pipeline->GetMissTable()->GetDesc().Width;
	dispatchDesc.MissShaderTable.StrideInBytes = m_Pipeline->GetShaderTableSize();
	dispatchDesc.RayGenerationShaderRecord.StartAddress = m_Pipeline->GetRayGenTable()->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = m_Pipeline->GetRayGenTable()->GetDesc().Width;
	dispatchDesc.Width = m_Width;
	dispatchDesc.Height = m_Height;
	dispatchDesc.Depth = 1;

	cmdList->SetPipelineState1(m_Pipeline->GetStateObject().Get());
	cmdList->DispatchRays(&dispatchDesc);
}

void Lighting::Update(float deltaTime)
{
}
