#include "pch.hpp"

#include <DXR/Renderer/Renderer.hpp>
#include <DXR/Renderer/Attributes/RaytracingPipeline.hpp>


class Basic : public Renderer
{
public:
	void InitializeSample() override;
	void RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList) override;
};


Renderer* CreateSample()
{
	return new Basic();
}

void Basic::InitializeSample()
{
	RaytracingPipelineDesc desc = {};
	desc.RayGenEntry.EntryName = L"RayGenMain";
	desc.HitGroups.emplace_back(L"ClosestHitMain", L"HitGroup", D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
	desc.MissShaders.emplace_back(L"MissMain");

	m_Pipeline = new RaytracingPipeline(desc);
}

void Basic::RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList)
{
}
