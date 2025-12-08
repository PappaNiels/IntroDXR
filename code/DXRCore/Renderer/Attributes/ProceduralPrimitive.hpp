#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <DirectXMath.h>

#include <DXRCore/Utils/Assert.hpp>

class ProceduralPrimitive
{
public:
	struct alignas(8) Entry
	{
		D3D12_RAYTRACING_AABB AABB;
		DirectX::XMFLOAT3 Position;
		float Padding;
	};
	
	void AddEntry(const Entry& entry);
	void BuildBLAS();
	
	D3D12_GPU_VIRTUAL_ADDRESS GetBLASAddress() const
	{
		return m_BLAS->GetGPUVirtualAddress();
	}

	void SetFlags(D3D12_RAYTRACING_GEOMETRY_FLAGS flags)
	{
		m_Flags = flags;
	}

	void SetHitGroupIndex(uint32_t hitGroupIndex)
	{
		m_HitGroupIdx = hitGroupIndex;
	}

private:
	friend class TLAS;

	std::vector<Entry> m_Entries;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_AABBs;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_BLAS;

	uint32_t m_HitGroupIdx = 0;
	D3D12_RAYTRACING_GEOMETRY_FLAGS m_Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
};

class ProceduralPrimitiveInstance
{
public:
	void SetMesh(ProceduralPrimitive* primitive)
	{
		m_ProceduralPrimitive = primitive;
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetBLASAddress() const
	{
		return m_ProceduralPrimitive->GetBLASAddress();
	}

	_declspec(property(put = SetRotation)) DirectX::XMFLOAT4 Rotation;
	_declspec(property(put = SetTranslation)) DirectX::XMFLOAT3 Translation;
	_declspec(property(put = SetScale)) DirectX::XMFLOAT3 Scale;

	void SetTranslation(const DirectX::XMFLOAT3& translation)
	{
		m_Translation = translation;
		IsDirty = true;
	}

	void SetRotation(const DirectX::XMFLOAT4& rotation)
	{
		m_Rotation = rotation;
		IsDirty = true;
	}

	void SetScale(const DirectX::XMFLOAT3& scale)
	{
		m_Scale = scale;
		IsDirty = true;
	}

	void SetReflectanceCoefficient(float reflectance)
	{
		m_Reflectance = reflectance;
	}

	void SetColor(const DirectX::XMFLOAT4& color)
	{
		m_Color = color;
	}

	DirectX::XMMATRIX GetMatrix() const
	{
		auto translation = DirectX::XMMatrixTranslation(m_Translation.x, m_Translation.y, m_Translation.z);
		auto rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&m_Rotation));
		auto scale = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);

		return translation * rotation * scale;
	}

	operator bool() const
	{
		return m_ProceduralPrimitive != nullptr;
	}

private:
	friend class TLAS;

	ProceduralPrimitive* m_ProceduralPrimitive;

	DirectX::XMFLOAT4 m_Rotation;
	DirectX::XMFLOAT3 m_Translation;
	DirectX::XMFLOAT3 m_Scale{ 1.0f, 1.0f, 1.0f };

	DirectX::XMFLOAT4 m_Color{ 1.0f, 1.0f, 1.0f, 1.0f };
	float m_Reflectance = 0.0f;

	bool IsDirty = true;
};
