#include "pch.hpp"

#include <DXR/Renderer/Renderer.hpp>
#include <DXR/Renderer/Attributes/TLAS.hpp>
#include <DXR/Renderer/Attributes/Mesh.hpp>

static std::wstring g_SampleName = L"Lighting";

class Lighting : public Renderer
{
	void InitializeSample() override;
	void RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList) override;


private:
	TLAS* m_TLAS = nullptr;
	Mesh* m_Mesh = nullptr;
};

SAMPLE(Lighting)

std::wstring Renderer::ms_SampleName = L"Lighting";

void Lighting::InitializeSample()
{
}

void Lighting::RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList)
{
}
