typedef BuiltInTriangleIntersectionAttributes MyAttributes;

#include "Shared.hpp"

struct RayPayload
{
    float4 Color;
};

StructuredBuffer<hlsl::Mesh> g_MeshData : register(t0);
RaytracingAccelerationStructure g_Scene : register(t1);

cbuffer Core : register(b0)
{
    float4x4 g_InverseViewProjection;
    float3 g_CameraPosition;
    uint g_UAV;
}

ConstantBuffer<hlsl::DirectionalLight> g_Light : register(b1);

RayDesc GetPrimaryRay(uint2 index)
{
    RayDesc ray;
    ray.Origin = g_CameraPosition;

    float2 screenPos = float2(index) + 0.5f.xx;
    screenPos = screenPos / (float2)DispatchRaysDimensions();
    screenPos = mad(screenPos, 2.0f, -1.0f);
    screenPos.y = -screenPos.y;

    float4 world = mul(g_InverseViewProjection, float4(screenPos, 0.0f, 1.0f));
    world.xyz /= world.w;

    ray.Direction = normalize(world.xyz - g_CameraPosition);
    ray.TMin = 0.01f;
    ray.TMax = 100.0f;
    
    return ray;
}

[shader("raygeneration")]
void RayGenMain()
{
    RWTexture2D<float4> RenderTarget = ResourceDescriptorHeap[g_UAV];
    
    RayDesc ray = GetPrimaryRay((uint2) DispatchRaysIndex());
    
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(g_Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
    
    RenderTarget[DispatchRaysIndex().xy] = payload.Color;
}

[shader("closesthit")]
void ClosestMain(inout RayPayload payload, in MyAttributes attr)
{
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    
    hlsl::Mesh mesh = g_MeshData[InstanceIndex()];
    
    ByteAddressBuffer indexBuffer = ResourceDescriptorHeap[mesh.IndexIdx]; // this needs to be flexible with 16bit and 32bit. byteaddressbuffer would work here :)
    StructuredBuffer<float3> normalBuffer = ResourceDescriptorHeap[mesh.NormalIdx];
    
    uint3 indices = indexBuffer.Load3(PrimitiveIndex() * 3 * 4);
    
    float3 normal = normalize(normalBuffer[indices.x] * barycentrics.x + normalBuffer[indices.y] * barycentrics.y + normalBuffer[indices.z] * barycentrics.z);
    
    float nDotl = dot(normal, -normalize(g_Light.Direction));
    float3 radiance = max(nDotl, 0.0f) * g_Light.Intensity * g_Light.Color;
    
    payload.Color = float4(mesh.Color.rgb * radiance + mesh.Color.rgb * 0.1f, 1.0f);
}

[shader("miss")]
void MissMain(inout RayPayload payload)
{
    payload.Color = float4(1.0f, 1.0f, 0.0f, 1.0f);
}