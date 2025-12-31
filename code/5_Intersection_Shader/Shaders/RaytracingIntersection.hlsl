typedef BuiltInTriangleIntersectionAttributes MyAttributes;

#include "Shared.hpp"

struct MyPrimitiveAttributes
{
    float3 Color;
};

struct RayPayload
{
    float4 Color;
    uint Depth;
};

struct RayPayloadShadow
{
    bool IsOccluded;
};

StructuredBuffer<hlsl:: Mesh> g_MeshData : register(t0);
RaytracingAccelerationStructure g_Scene : register(t1);

cbuffer Core : register(b0)
{
    float4x4 g_InverseViewProjection;
    float4x4 g_Transform;
    float4x4 g_TransformInverse;
    float3 g_CameraPosition;
    uint g_UAV;
    uint g_SkySRV;
}

ConstantBuffer<hlsl:: DirectionalLight> g_Light : register(b1);

SamplerState g_LinearSampler : register(s0);

static const float INVPI = 0.31830988618379067153777f;
static const float INV2PI = 0.15915494309189533576888f;
static const float PI = 3.14159265;

RayDesc GetPrimaryRay(uint2 index)
{
    RayDesc ray;
    ray.Origin = g_CameraPosition;

    float2 screenPos = float2(index) + 0.5f.xx;
    screenPos = screenPos / (float2) DispatchRaysDimensions();
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
    
    RayPayload payload = { float4(0, 0, 0, 0), 0 };
    TraceRay(g_Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
    
    RenderTarget[DispatchRaysIndex().xy] = payload.Color;
}

[shader("closesthit")]
void ClosestMain(inout RayPayload payload, in MyAttributes attr)
{
    if (payload.Depth + 1 >= MAX_RECURSION)
    {
        payload.Color = float4(0.0f.xxx, 1.0f);
        return;
    }
    
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    
    hlsl::Mesh mesh = g_MeshData[InstanceIndex()];
    
    ByteAddressBuffer indexBuffer = ResourceDescriptorHeap[mesh.IndexIdx]; // this needs to be flexible with 16bit and 32bit. byteaddressbuffer would work here :)
    StructuredBuffer<float3> normalBuffer = ResourceDescriptorHeap[mesh.NormalIdx];
    
    uint3 indices = indexBuffer.Load3(PrimitiveIndex() * 3 * 4);
    
    // Interpolate the normal from the three vertices
    float3 normal = normalize(normalBuffer[indices.x] * barycentrics.x + normalBuffer[indices.y] * barycentrics.y + normalBuffer[indices.z] * barycentrics.z);
    
    // We just shade the cube and do not cast a shadow ray. Since we dont have anything else in the scene, that would be stupid...
    float nDotl = dot(normal, -normalize(g_Light.Direction));
    float3 radiance = max(nDotl, 0.0f) * g_Light.Intensity * g_Light.Color;
    
    RayPayloadShadow shadowPayload = { true };
    
    RayDesc shadowRay;
    shadowRay.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    shadowRay.Direction = -normalize(g_Light.Direction);
    shadowRay.TMin = 0.01f;
    shadowRay.TMax = 1000.0f;
    
    TraceRay(g_Scene, RAY_FLAG_CULL_FRONT_FACING_TRIANGLES | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, ~0, 0, 1, 1, shadowRay, shadowPayload);
    
    float shadowValue = shadowPayload.IsOccluded ? 0.0f : 1.0f;
    
    RayPayload radiancePayload = { float4(0.0f.xxx, 1.0f), ++payload.Depth };
    
    if (mesh.Reflectance > 0.001)
    {
        // Note: We use the same closest hit and miss shaders as the regular pipeline
        RayDesc ray;
        ray.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
        ray.Direction = reflect(WorldRayDirection(), normal);
        ray.TMin = 0.01f;
        ray.TMax = 1000.0f;
    
        TraceRay(g_Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, radiancePayload);
        
        float cosi = saturate(dot(-WorldRayDirection(), normal));
        float3 f0 = mesh.Color.rgb + (1 - mesh.Color.rgb) * pow(1 - cosi, 5);
        
        radiancePayload.Color.rgb *= mesh.Reflectance * f0;
    }
    
    
    radiance = mesh.Color.rgb * radiance * 0.65f + radiancePayload.Color.rgb;
    
    payload.Color = float4(radiance * shadowValue + mesh.Color.rgb * 0.35f, 1.0f);
}

[shader("closesthit")]
void ClosestMainPrimitive(inout RayPayload payload, in MyPrimitiveAttributes attr)
{
    payload.Color = float4(attr.Color, 1.0f);
}

[shader("miss")]
void MissMain(inout RayPayload payload)
{
    Texture2D<float4> sky = ResourceDescriptorHeap[g_SkySRV];
    
    // Calculate the uvs 
    float3 rayDirection = WorldRayDirection();
    
    float phi = atan2(rayDirection.y, rayDirection.x) + PI;
    float theta = acos(-rayDirection.z);
    
    float u = phi / (2 * PI);
    float v = 1.0 - (theta / PI);
    
    payload.Color = float4(sky.SampleLevel(g_LinearSampler, float2(u, v), 0).rgb, 1.0f);
}

[shader("miss")]
void MissMainShadow(inout RayPayloadShadow payload)
{
    payload.IsOccluded = false;
}

[shader("intersection")]
void IntersectionMainSphere()
{
    // https://raytracing.github.io/books/RayTracingInOneWeekend.html#addingasphere/ray-sphereintersection
    
    float3 oc = 1.5f.xxx - WorldRayOrigin();
    float a = dot(WorldRayDirection(), WorldRayDirection());
    float h = dot(WorldRayDirection(), oc);
    float c = dot(oc, oc) - 0.5f * 0.5f; // radius
    float discriminant = h * h - a * c;

    if (discriminant >= 0.0f)
    {
        MyPrimitiveAttributes attr;
        attr.Color = 1.0f.xxx;
        
        float t = (h - sqrt(discriminant)) / a;
        float3 normal = normalize((WorldRayDirection() * t + WorldRayOrigin()) - 1.5f.xxx);
        
        attr.Color = mad(normal, 0.5, 0.5);
        
        ReportHit(t, 0, attr);
    }
}

[shader("intersection")]
void IntersectionMainTorus()
{
    float3 ro = mul(g_TransformInverse, float4(ObjectRayOrigin(), 1.0f)).xyz;
    float3 rd = mul(g_TransformInverse, float4(ObjectRayDirection(), 0.0f)).xyz;
    
    float2 tor = float2(0.3f, 0.1f);
    
    float po = 1.0;
    float Ra2 = tor.x * tor.x;
    float ra2 = tor.y * tor.y;
    float m = dot(ro, ro);
    float n = dot(ro, rd);
    float k = (m + Ra2 - ra2) / 2.0;
    float k3 = n;
    float k2 = n * n - Ra2 * dot(rd.xy, rd.xy) + k;
    float k1 = n * k - Ra2 * dot(rd.xy, ro.xy);
    float k0 = k * k - Ra2 * dot(ro.xy, ro.xy);
    
    if (abs(k3 * (k3 * k3 - k2) + k1) < 0.01)
    {
        po = -1.0;
        float tmp = k1;
        k1 = k3;
        k3 = tmp;
        k0 = 1.0 / k0;
        k1 = k1 * k0;
        k2 = k2 * k0;
        k3 = k3 * k0;
    }
    
    float c2 = k2 * 2.0 - 3.0 * k3 * k3;
    float c1 = k3 * (k3 * k3 - k2) + k1;
    float c0 = k3 * (k3 * (c2 + 2.0 * k2) - 8.0 * k1) + 4.0 * k0;
    c2 /= 3.0;
    c1 *= 2.0;
    c0 /= 3.0;
    float Q = c2 * c2 + c0;
    float R = c2 * c2 * c2 - 3.0 * c2 * c0 + c1 * c1;
    float h = R * R - Q * Q * Q;
    
    if (h >= 0.0)
    {
        MyPrimitiveAttributes attr;
        attr.Color = 1.0f.xxx;
        
        h = sqrt(h);
        float v = sign(R + h) * pow(abs(R + h), 1.0 / 3.0); // cube root
        float u = sign(R - h) * pow(abs(R - h), 1.0 / 3.0); // cube root
        float2 s = float2((v + u) + 4.0 * c2, (v - u) * sqrt(3.0));
        float y = sqrt(0.5 * (length(s) + s.x));
        float x = 0.5 * s.y / y;
        float r = 2.0 * c1 / (x * x + y * y);
        float t1 = x - r - k3;
        t1 = (po < 0.0) ? 2.0 / t1 : t1;
        float t2 = -x - r - k3;
        t2 = (po < 0.0) ? 2.0 / t2 : t2;
        float t = 1e20;
        if (t1 > 0.0)
            t = t1;
        if (t2 > 0.0)
            t = min(t, t2);
        
        ReportHit(t, 0, attr);
    }
    
    float sQ = sqrt(Q);
    float w = sQ * cos(acos(-R / (sQ * Q)) / 3.0);
    float d2 = -(w + c2);
    if (d2 > 0.0)
    {
        MyPrimitiveAttributes attr;
        attr.Color = 1.0f.xxx;
        
        float d1 = sqrt(d2);
        float h1 = sqrt(w - 2.0 * c2 + c1 / d1);
        float h2 = sqrt(w - 2.0 * c2 - c1 / d1);
        float t1 = -d1 - h1 - k3;
        t1 = (po < 0.0) ? 2.0 / t1 : t1;
        float t2 = -d1 + h1 - k3;
        t2 = (po < 0.0) ? 2.0 / t2 : t2;
        float t3 = d1 - h2 - k3;
        t3 = (po < 0.0) ? 2.0 / t3 : t3;
        float t4 = d1 + h2 - k3;
        t4 = (po < 0.0) ? 2.0 / t4 : t4;
        float t = 1e20;
        if (t1 > 0.0)
            t = t1;
        if (t2 > 0.0)
            t = min(t, t2);
        if (t3 > 0.0)
            t = min(t, t3);
        if (t4 > 0.0)
            t = min(t, t4);
        
        ReportHit(t, 0, attr);
    }
}