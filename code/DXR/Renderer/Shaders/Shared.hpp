#pragma once

#ifdef __cplusplus
#include <DirectXMath.h>

using float4x4 = DirectX::XMMATRIX;
using float4 = DirectX::XMVECTOR;

using uint = uint32_t;

#endif

struct Camera
{
    float4x4 InverseViewProj;
    float4 Position;
};

struct Model
{
    struct Mesh
    {
        uint Normals;
        uint UV0;
    } Mesh;

    struct Material
    {
        uint Color;
    } Material;
};
