#include "pch.hpp"

#include <DXRCore/Renderer/Renderer.hpp>

#include <DXRCore/Renderer/Attributes/TLAS.hpp>
#include <DXRCore/Renderer/Attributes/Mesh.hpp>
#include <DXRCore/Renderer/Attributes/ProceduralPrimitive.hpp>
#include <DXRCore/Renderer/Attributes/RaytracingPipeline.hpp>
#include <DXRCore/Renderer/Attributes/Texture.hpp>

#include <DirectXMath.h>

#if defined _DEBUG
#include "Shaders/RaytracingIntersection_Debug.hpp"
#else 
#include "Shaders/RaytracingIntersection_Release.hpp"
#endif

#include <DXRCore/Utils/Error.hpp>

#include <Shaders/Shared.hpp>

using namespace DirectX;

class Intersection : public Renderer
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
	ProceduralPrimitive* m_ProceduralPrimitive = nullptr;
	ProceduralPrimitive* m_ProceduralPrimitiveTorus = nullptr;

	MeshInstance* m_MeshInstance[2] = { nullptr, nullptr };
	ProceduralPrimitiveInstance* m_ProceduralPrimitiveInstance = nullptr;
	ProceduralPrimitiveInstance* m_ProceduralPrimitiveInstanceTorus = nullptr;

	Texture* m_SkyDome = nullptr;

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

SAMPLE(Intersection)

void Intersection::InitializeSample()
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

	m_MeshInstance[0] = new MeshInstance();
	m_MeshInstance[0]->SetMesh(m_Mesh);
	m_MeshInstance[0]->SetColor({0.5f, 1.0f, 0.5f, 1.0f});
	m_MeshInstance[0]->SetReflectanceCoefficient(0.0f);
	m_MeshInstance[0]->Translation = XMFLOAT3(0.0f, 0.0f, 1.5f);

	m_MeshInstance[1] = new MeshInstance();
	m_MeshInstance[1]->SetMesh(m_Mesh);
	m_MeshInstance[1]->SetColor({ 1.0f, 0.0f, 1.0f, 1.0f });
	m_MeshInstance[1]->SetReflectanceCoefficient(0.1f);
	m_MeshInstance[1]->Scale = XMFLOAT3(5.0f, 5.0f, 0.5f);

	m_ProceduralPrimitive = new ProceduralPrimitive();

	D3D12_RAYTRACING_AABB aabb = {};
	aabb.MinX = 1.0f;
	aabb.MinY = 1.0f;
	aabb.MinZ = 1.0f;

	aabb.MaxX = 2.0f;
	aabb.MaxY = 2.0f;
	aabb.MaxZ = 2.0f;

	m_ProceduralPrimitive->AddEntry({ aabb, {} });
	m_ProceduralPrimitive->SetHitGroupIndex(1);
	m_ProceduralPrimitive->BuildBLAS();

	m_ProceduralPrimitiveTorus = new ProceduralPrimitive();

	aabb = {};
	aabb.MinX = 2.0f;
	aabb.MinY = 2.0f;
	aabb.MinZ = 2.0f;

	aabb.MaxX = 3.0f;
	aabb.MaxY = 3.0f;
	aabb.MaxZ = 3.0f;

	m_ProceduralPrimitiveTorus->AddEntry({ aabb, {} });
	m_ProceduralPrimitiveTorus->SetHitGroupIndex(2);
	m_ProceduralPrimitiveTorus->BuildBLAS();

	m_ProceduralPrimitiveInstance = new ProceduralPrimitiveInstance();
	m_ProceduralPrimitiveInstance->SetMesh(m_ProceduralPrimitive);
	
	m_ProceduralPrimitiveInstanceTorus = new ProceduralPrimitiveInstance();
	m_ProceduralPrimitiveInstanceTorus->SetMesh(m_ProceduralPrimitiveTorus);

	m_TLAS = new TLAS();
	m_TLAS->AddMesh(m_MeshInstance[0]);
	m_TLAS->AddMesh(m_MeshInstance[1]);
	m_TLAS->AddProceduralPrimitive(m_ProceduralPrimitiveInstance);
	m_TLAS->AddProceduralPrimitive(m_ProceduralPrimitiveInstanceTorus);
	m_TLAS->Build();

	RaytracingPipelineDesc desc = {};
	desc.RayGenEntry.EntryName = L"RayGenMain";
	desc.HitGroups.emplace_back(L"ClosestMain", L"", L"", L"HitGroup", D3D12_HIT_GROUP_TYPE_TRIANGLES);
	desc.HitGroups.emplace_back(L"ClosestMainPrimitive", L"", L"IntersectionMainSphere", L"HitGroupPrimitive", D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
	desc.HitGroups.emplace_back(L"ClosestMainPrimitive", L"", L"IntersectionMainTorus", L"HitGroupPrimitiveTorus", D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
	desc.MissShaders.emplace_back(L"MissMain");
	desc.MissShaders.emplace_back(L"MissMainShadow");
	desc.ShaderCode.pShaderBytecode = g_RaytracingIntersection;
	desc.ShaderCode.BytecodeLength = sizeof(g_RaytracingIntersection);
	desc.RecursionDepth = MAX_RECURSION;

	// NOTE: we use a larger attribute size for the AABB intersection shader. we need to pass more data than just the barycentrics.
	desc.AttributeSize = sizeof(float) * 3; 
	desc.PayloadSize = sizeof(float) * 4 + sizeof(uint);

	CD3DX12_ROOT_PARAMETER params[Count] = {};
	params[Core].InitAsConstants(53, 0);
	params[Light].InitAsConstants(sizeof(hlsl::DirectionalLight) / sizeof(uint32_t), 1);
	params[GeometryData].InitAsShaderResourceView(0);
	params[BVH].InitAsShaderResourceView(1);

	CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT);

	desc.RootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(params), params, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);

	m_Pipeline = new RaytracingPipeline(desc);

	m_Camera = static_cast<Intersection::Camera*>(_aligned_malloc(sizeof(Intersection::Camera), alignof(Intersection::Camera)));

	if (m_Camera == nullptr)
	{
		FatalError("Failed to allocate memory for the camera");
		return;
	}

	m_Camera->Position = XMVectorSet(0.0f, -7.0f, 6.0f, 1.0f);
	m_Camera->Yaw = 90.0f;
	m_Camera->Pitch = -30.0f;

	m_DirectionalLight.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_DirectionalLight.Direction = XMFLOAT3(-0.25f, -0.25f, -0.5f);
	m_DirectionalLight.Intensity = 1.0f;

	m_SkyDome = new Texture("../assets/skydome/golden_gate_hills_2k.hdr");
}


void Intersection::RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList)
{
	cmdList->SetComputeRootSignature(m_Pipeline->GetRootSignature().Get());

	cmdList->SetComputeRoot32BitConstants(Core, 16, &m_Camera->InverseViewProjection, 0);

	auto transform = XMMatrixTranslation(2.5f, 2.5f, 2.5f);
	auto transformInverse = XMMatrixInverse(nullptr, transform);

	cmdList->SetComputeRoot32BitConstants(Core, 16, &transform, 16);
	cmdList->SetComputeRoot32BitConstants(Core, 16, &transformInverse, 32);

	cmdList->SetComputeRoot32BitConstants(Core, 3, &m_Camera->Position, 48);

	cmdList->SetComputeRoot32BitConstant(Core, m_UAV, 51);
	cmdList->SetComputeRoot32BitConstant(Core, m_SkyDome->GetSRV(), 52);

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

void Intersection::Update(float deltaTime)
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
