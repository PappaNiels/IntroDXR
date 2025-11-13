typedef BuiltInTriangleIntersectionAttributes MyAttributes;

struct RayPayload
{
    float4 Color;
};

RaytracingAccelerationStructure Scene : register(t0);

cbuffer RenderTarget : register(b0)
{
    uint g_UAV;
}

float3 LinearToSRGB(float3 color)
{
    return pow(color, 1.0 / 2.2);
}

[shader("raygeneration")]
void RayGenMain()
{
    RWTexture2D<float4> RenderTarget = ResourceDescriptorHeap[g_UAV];
    
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
    
    RenderTarget[DispatchRaysIndex().xy] = payload.Color;
}

[shader("closesthit")]
void ClosestMain(inout RayPayload payload, in MyAttributes attr)
{
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    
    payload.Color = float4(LinearToSRGB(barycentrics), 1.0f.x);
}

[shader("miss")]
void MissMain(inout RayPayload payload)
{
    payload.Color = float4(1.0f, 1.0f, 0.0f, 1.0f);
}