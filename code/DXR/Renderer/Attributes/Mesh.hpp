#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <type_traits>

#include <DirectXMath.h>

#include <DXR/Utils/Assert.hpp>

struct MeshInstance
{
	class Mesh* Mesh;
	DirectX::XMFLOAT4 Rotation;
	DirectX::XMFLOAT3 Translation;
	DirectX::XMFLOAT3 Scale;

	bool IsDirty = true;
};

class Mesh
{
public:
	Mesh() = default;

	template<typename T>
	void SetPositionBuffer(uint64_t numPositions, const T* data);

	template<typename T>
	void SetNormalBuffer(uint64_t numNormals, const T* data);

	template<typename T>
	void SetUV0Buffer(uint64_t numUV0, const T* data);

	template<typename T>
	void SetIndexBuffer(uint64_t numIndices, const T* data);

	void BuildBLAS();

	D3D12_GPU_VIRTUAL_ADDRESS GetBLASAddress() const
	{
		return m_BLAS->GetGPUVirtualAddress();
	}

private:
	void SetBufferData(Microsoft::WRL::ComPtr<ID3D12Resource>& buffer, uint64_t numComponents, uint64_t componentSize, const void* data);

	Microsoft::WRL::ComPtr<ID3D12Resource> m_PositionBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_NormalBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UV0Buffer;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_BLAS;

	uint64_t m_VertexCount = static_cast<uint64_t>(-1);
	uint64_t m_IndexCount = static_cast<uint64_t>(-1);

	uint32_t m_NormalSRV = static_cast<uint32_t>(-1);
	uint32_t m_UV0SRV = static_cast<uint32_t>(-1);

	uint32_t m_IndexSize = static_cast<uint32_t>(-1);
	D3D12_RAYTRACING_GEOMETRY_FLAGS m_Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
};

template<typename T>
inline void Mesh::SetPositionBuffer(uint64_t numPositions, const T* data)
{
	static_assert(sizeof(T) == 3 * sizeof(float), "Size of the positions is not correct. It must be 3 32-bit floating point values");
	static_assert(std::is_floating_point_v<decltype(T::x)>, "Type of the positions is not a floating point value");

	if (m_VertexCount == static_cast<uint64_t>(-1))
	{
		m_VertexCount = numPositions;
	}

	ASSERT(m_VertexCount == numPositions, "There are too few/too many positions compared to the current set vertex count");

	SetBufferData(m_PositionBuffer, numPositions, sizeof(T), data);
}

template<typename T>
inline void Mesh::SetNormalBuffer(uint64_t numNormals, const T* data)
{
	static_assert(sizeof(T) == 3 * sizeof(float), "Size of the normals is not correct. It must be 3 32-bit floating point values (12 bytes)");
	static_assert(std::is_floating_point_v<decltype(T::x)>, "Type of the normals is not a floating point value");

	if (m_VertexCount == static_cast<uint64_t>(-1))
	{
		m_VertexCount = numNormals;
	}

	ASSERT(m_VertexCount == numNormals, "There are too few/too many normals compared to the current set vertex count");

	SetBufferData(m_NormalBuffer, numNormals, sizeof(T), data);
}

template<typename T>
inline void Mesh::SetUV0Buffer(uint64_t numUV0, const T* data)
{
	static_assert(sizeof(T) == 2 * sizeof(float), "Size of the uv0s is not correct. It must be 3 32-bit floating point values (12 bytes)");
	static_assert(std::is_floating_point_v<decltype(T::x)>, "Type of the uv0s is not a floating point value");

	if (m_VertexCount == static_cast<uint64_t>(-1))
	{
		m_VertexCount = numUV0;
	}

	ASSERT(m_VertexCount == numUV0, "There are too few/too many uv0s compared to the current set vertex count");

	SetBufferData(m_UV0Buffer, numUV0, sizeof(T), data);
}

template<typename T>
inline void Mesh::SetIndexBuffer(uint64_t numIndices, const T* data)
{
	static_assert(sizeof(T) == sizeof(uint16_t) || sizeof(T) == sizeof(uint32_t), "The index side needs to be either 16-bit or 32-bit");

	m_IndexCount = numIndices;
	m_IndexSize = sizeof(T);

	SetBufferData(m_IndexBuffer, numIndices, sizeof(T), data);
}
