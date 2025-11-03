#pragma once

#include "Shared.hpp"

ConstantBuffer<Camera> g_Camera : register(b0);

RaytracingAccelerationStructure g_TLAS : register(t0);

RWTexture2D<float4> g_RenderTarget : register(u0);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;