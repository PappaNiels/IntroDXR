#define SHOW_FIRST_LANE 1
#define SHOW_ACTIVE_LANES 1

RWTexture2D<float4> Output : register(u0);

[numthreads(16, 16, 1)]
void main(uint2 dispatchID : SV_DispatchThreadID)
{
#if SHOW_UV
    uint2 Dimensions = 0.0.xx;
    Output.GetDimensions(Dimensions.x, Dimensions.y);
    
    Output[dispatchID] = float4(dispatchID.x / float(Dimensions.x), dispatchID.y / float(Dimensions.y), 0.0f, 1.0f);
#elif SHOW_FIRST_LANE
    if (WaveIsFirstLane())
    {
        Output[dispatchID] = 1.0f.xxxx;
    }
    else if (WaveGetLaneIndex() == WaveActiveMax(WaveGetLaneIndex()))
    {
        
        Output[dispatchID] = float4(1.0f, 0.0f.xx, 1.0f);
    }
    else
    {
        Output[dispatchID] = float4(0.0f.xxx, 1.0f);
    }
#elif SHOW_ACTIVE_LANES
    uint4 activeLaneMask = WaveActiveBallot(true);
    uint numActiveLanes = countbits(activeLaneMask.x) + countbits(activeLaneMask.y) + countbits(activeLaneMask.z) + countbits(activeLaneMask.w);
    float activeRatio = (float)numActiveLanes / float(32);
    Output[dispatchID] = float4(activeRatio, activeRatio, activeRatio, 1.0);
#endif
}