//#include "RaytracingCommon.hlsli"

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

RaytracingAccelerationStructure Scene : register(t0, space0);

struct RayPayload
{
    float4 color;
};

struct RayPayloadShadow
{
    bool IsOccluded;
};

cbuffer t : register(b0)
{
    uint UAV;
}

[shader("raygeneration")]
void RayGenMain()
{
    RWTexture2D<float4> RenderTarget = ResourceDescriptorHeap[UAV];
    
    float2 lerpValues = (float2) DispatchRaysIndex() / (float2) DispatchRaysDimensions(); // 1280, 720 -> [0..1]
    
    float3 rayDir = float3(0, 0, 1);
    float3 origin = float3(
        lerp(-1.0f, 1.0f, lerpValues.x),
        lerp(-1.0f, 1.0f, lerpValues.y),
        0.0f);
    
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
    
    RayPayloadShadow s = { false };
    
    
    RenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void ClosestMain(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = 1.0f.xxxx;
}

[shader("miss")]
void MissMain(inout RayPayload payload)
{
    payload.color = float4(1.0f, 1.0f, 0.0f, 1.0f);
}