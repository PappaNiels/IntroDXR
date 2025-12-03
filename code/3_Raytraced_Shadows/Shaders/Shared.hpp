#pragma once

#if __cplusplus
#include <cstdint>
#include <DirectXMath.h>

using uint = uint32_t;

using float4x4 = DirectX::XMFLOAT4X4;

using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

#endif

namespace hlsl
{
	struct Camera
	{
		float4x4 InverseViewProjection;
		float3 Position;
	};

	struct Mesh
	{
		float4 Color;
		float Reflectance;

		uint IndexIdx;
		uint NormalIdx;
		uint UV0Idx;
	};

	struct DirectionalLight
	{
		float3 Direction;
		float Intensity;
		float3 Color;
	};
}