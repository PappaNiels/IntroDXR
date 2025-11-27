#include "pch.hpp"

#include <DXRCore/Renderer/Renderer.hpp>

#include <DXRCore/Renderer/Attributes/TLAS.hpp>
#include <DXRCore/Renderer/Attributes/Mesh.hpp>
#include <DXRCore/Renderer/Attributes/RaytracingPipeline.hpp>

#include <DirectXMath.h>

#if defined _DEBUG
#include "Shaders/RaytracingLighting_Debug.hpp"
#else 
#include "Shaders/RaytracingLighting_Release.hpp"
#endif

#include <DXRCore/Utils/Error.hpp>

#include <Shaders/Shared.hpp>

using namespace DirectX;

class Lighting : public Renderer
{
	enum
	{
		Core,
		Light,
		GeometryData,
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

	struct alignas(16) Camera
	{
		XMMATRIX InverseViewProjection;
		XMVECTOR Position;
		XMVECTOR Forward;

		float Yaw;
		float Pitch;

		void CalculateForward()
		{
			XMFLOAT3 dir;

			dir.x = cos(XMConvertToRadians(Yaw)) * cos(XMConvertToRadians(Pitch));
			dir.y = sin(XMConvertToRadians(Yaw)) * cos(XMConvertToRadians(Pitch));
			dir.z = sin(XMConvertToRadians(Pitch));

			Forward = XMLoadFloat3(&dir);
			Forward = XMVector3Normalize(Forward);
		}

	}*m_Camera = nullptr;

	hlsl::DirectionalLight m_DirectionalLight;
};

SAMPLE(Lighting)

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

	XMFLOAT3 normals[] = {
		XMFLOAT3(0.0f,  0.0f, +1.0f),
		XMFLOAT3(0.0f,  0.0f, +1.0f),
		XMFLOAT3(0.0f,  0.0f, +1.0f),
		XMFLOAT3(0.0f,  0.0f, +1.0f),

		XMFLOAT3(0.0f,  0.0f, -1.0f),
		XMFLOAT3(0.0f,  0.0f, -1.0f),
		XMFLOAT3(0.0f,  0.0f, -1.0f),
		XMFLOAT3(0.0f,  0.0f, -1.0f),

		XMFLOAT3(-1.0f,  0.0f,  0.0f),
		XMFLOAT3(-1.0f,  0.0f,  0.0f),
		XMFLOAT3(-1.0f,  0.0f,  0.0f),
		XMFLOAT3(-1.0f,  0.0f,  0.0f),

		XMFLOAT3(+1.0f,  0.0f,  0.0f),
		XMFLOAT3(+1.0f,  0.0f,  0.0f),
		XMFLOAT3(+1.0f,  0.0f,  0.0f),
		XMFLOAT3(+1.0f,  0.0f,  0.0f),

		XMFLOAT3(0.0f, +1.0f,  0.0f),
		XMFLOAT3(0.0f, +1.0f,  0.0f),
		XMFLOAT3(0.0f, +1.0f,  0.0f),
		XMFLOAT3(0.0f, +1.0f,  0.0f),

		XMFLOAT3(0.0f, -1.0f,  0.0f),
		XMFLOAT3(0.0f, -1.0f,  0.0f),
		XMFLOAT3(0.0f, -1.0f,  0.0f),
		XMFLOAT3(0.0f, -1.0f,  0.0f)
	};

	uint32_t indices[] = {
		0,  2,  1,  0,  3,  2,
		4,  6,  5,  4,  7,  6,
		8,  10, 9,  8, 11, 10,
		12, 14, 13, 12, 15, 14,
		16, 18, 17, 16, 19, 18,
		20, 22, 21, 20, 23, 22
	};

	m_Mesh = new Mesh();
	m_Mesh->SetPositionBuffer(_countof(positions), positions);
	m_Mesh->SetNormalBuffer(_countof(normals), normals);
	m_Mesh->SetIndexBuffer(_countof(indices), indices);
	m_Mesh->BuildBLAS();

	m_MeshInstance = new MeshInstance();
	m_MeshInstance->SetMesh(m_Mesh);

	m_TLAS = new TLAS();
	m_TLAS->AddMesh(m_MeshInstance);
	m_TLAS->Build();

	RaytracingPipelineDesc desc = {};
	desc.RayGenEntry.EntryName = L"RayGenMain";
	desc.HitGroups.emplace_back(L"ClosestMain", L"", L"", L"HitGroup", D3D12_HIT_GROUP_TYPE_TRIANGLES);
	desc.MissShaders.emplace_back(L"MissMain");
	desc.ShaderCode.pShaderBytecode = g_RaytracingLighting;
	desc.ShaderCode.BytecodeLength = sizeof(g_RaytracingLighting);
	desc.RecursionDepth = 1;

	desc.AttributeSize = sizeof(float) * 2;
	desc.PayloadSize = sizeof(float) * 4;

	CD3DX12_ROOT_PARAMETER params[Count] = {};
	params[Core].InitAsConstants(20, 0);
	params[Light].InitAsConstants(sizeof(hlsl::DirectionalLight) / sizeof(uint32_t), 1);
	params[GeometryData].InitAsShaderResourceView(0);
	params[BVH].InitAsShaderResourceView(1);

	desc.RootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(params), params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);

	m_Pipeline = new RaytracingPipeline(desc);

	m_Camera = static_cast<Lighting::Camera*>(_aligned_malloc(sizeof(Lighting::Camera), alignof(Lighting::Camera)));

	if (m_Camera == nullptr)
	{
		FatalError("Failed to allocate memory for the camera");
		return;
	}

	m_Camera->Position = XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f);
	m_Camera->Yaw = 90.0f;
	m_Camera->Pitch = 0.0f;

	m_DirectionalLight.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_DirectionalLight.Direction = XMFLOAT3(0.3f, 0.5f, -0.2f);
	m_DirectionalLight.Intensity = 1.0f;
}


void Lighting::RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList)
{
	//m_TLAS->Build(cmdList);

	cmdList->SetComputeRootSignature(m_Pipeline->GetRootSignature().Get());

	cmdList->SetComputeRoot32BitConstants(Core, 16, &m_Camera->InverseViewProjection, 0);
	cmdList->SetComputeRoot32BitConstants(Core, 3, &m_Camera->Position, 16);
	cmdList->SetComputeRoot32BitConstants(Core, 1, &m_UAV, 19);

	cmdList->SetComputeRoot32BitConstants(Light, sizeof(hlsl::DirectionalLight) / sizeof(uint32_t), &m_DirectionalLight, 0);

	cmdList->SetComputeRootShaderResourceView(GeometryData, m_TLAS->GetGeometryData());

	cmdList->SetComputeRootShaderResourceView(BVH, m_TLAS->GetVirtualAddress());

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
	static float time = deltaTime;
	time += deltaTime;

	const static XMVECTOR forward = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const static XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	const static XMVECTOR up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	XMVECTOR dT = XMVectorSet(deltaTime, deltaTime, deltaTime, 0.0f);

	if (GetAsyncKeyState('W') & 0x8000)
	{
		m_Camera->Position += XMVectorMultiply(m_Camera->Forward, dT);
	}

	if (GetAsyncKeyState('S') & 0x8000)
	{
		m_Camera->Position -= XMVectorMultiply(m_Camera->Forward, dT);
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		m_Camera->Position += XMVectorMultiply(XMVector3Cross(m_Camera->Forward, up), dT);
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		m_Camera->Position -= XMVectorMultiply(XMVector3Cross(m_Camera->Forward, up), dT);
	}

	if (GetAsyncKeyState('Q') & 0x8000)
	{
		m_Camera->Position += XMVectorMultiply(up, dT);
	}

	if (GetAsyncKeyState('E') & 0x8000)
	{
		m_Camera->Position -= XMVectorMultiply(up, dT);
	}

	constexpr float rotateSpeed = 15.0f;

	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		m_Camera->Pitch += deltaTime * rotateSpeed;
		m_Camera->Pitch = std::min(m_Camera->Pitch, 89.0f);
	}

	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		m_Camera->Pitch -= deltaTime * rotateSpeed;
		m_Camera->Pitch = std::max(m_Camera->Pitch, -89.0f);
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		m_Camera->Yaw += deltaTime * rotateSpeed;
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		m_Camera->Yaw -= deltaTime * rotateSpeed;
	}

	m_Camera->CalculateForward();

	auto view = XMMatrixLookAtRH(m_Camera->Position, m_Camera->Position + XMVector3Normalize(m_Camera->Forward), up);
	auto projection = XMMatrixPerspectiveFovRH(XMConvertToRadians(55.0f), static_cast<float>(m_Width) / m_Height, 0.01f, 100.0f);
	auto vp = XMMatrixMultiply(view, projection);

	m_Camera->InverseViewProjection = XMMatrixInverse(nullptr, vp);
}
