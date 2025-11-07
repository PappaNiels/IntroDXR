typedef BuiltInTriangleIntersectionAttributes MyAttributes;

#include "Shared.hpp"

StructuredBuffer<hlsl::Mesh> ModelData : register(t0);
RaytracingAccelerationStructure Scene : register(t1);

struct RayPayload
{
    float4 color;
};

struct RayPayloadShadow
{
    bool IsOccluded;
};

cbuffer RenderTarget : register(b0)
{
    float4x4 InverseViewProjection;
    float3 CameraPosition;
    uint UAV;
}

RayDesc GetPrimaryRay(uint2 index)
{
    RayDesc ray;
    ray.Origin = CameraPosition;

    float2 screenPos = float2(index) + 0.5f.xx;
    screenPos = screenPos / (float2)DispatchRaysDimensions();
    screenPos = mad(screenPos, 2.0f, -1.0f);
    screenPos.y = -screenPos.y;

    float4 world = mul(InverseViewProjection, float4(screenPos, 0.0f, 1.0f));
    world.xyz /= world.w;

    ray.Direction = normalize(world.xyz - CameraPosition);
    ray.TMin = 0.01f;
    ray.TMax = 100.0f;
    
    return ray;
}

[shader("raygeneration")]
void RayGenMain()
{
    RWTexture2D<float4> RenderTarget = ResourceDescriptorHeap[UAV];
    
    RayDesc ray = GetPrimaryRay((uint2) DispatchRaysIndex());
    
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
    
    RenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void ClosestMain(inout RayPayload payload, in MyAttributes attr)
{
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    
    hlsl::Mesh model = ModelData[InstanceIndex()];
    
    ByteAddressBuffer indexBuffer = ResourceDescriptorHeap[model.IndexIdx]; // this needs to be flexible with 16bit and 32bit. byteaddressbuffer would work here :)
    StructuredBuffer<float3> normalBuffer = ResourceDescriptorHeap[model.NormalIdx];
    
    uint3 indices = indexBuffer.Load3(PrimitiveIndex() * 3 * 4);
    float3 normal = normalize(normalBuffer[indices.x] * barycentrics.x + normalBuffer[indices.y] * barycentrics.y + normalBuffer[indices.z] * barycentrics.z);
    
    payload.color = float4(mad(normal, 0.5, 0.5), 1.0f);
    //payload.color = float4(model.Color.rgb, 1.0f);
}

[shader("miss")]
void MissMain(inout RayPayload payload)
{
    payload.color = float4(1.0f, 1.0f, 0.0f, 1.0f);
}