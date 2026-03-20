#include "pch.hpp"

#include <DXRCore/Renderer/Renderer.hpp>

#include <DXRCore/Renderer/Attributes/DescriptorHeap.hpp>
#include <DXRCore/Renderer/Attributes/Texture.hpp>
#include <DXRCore/Renderer/Attributes/ComputePipeline.hpp>

#include <DXRCore/Utils/Error.hpp>

#include <DirectXMath.h>

#if defined _DEBUG
#include "Shaders/HelloCompute_Debug.hpp"
#else 
#include "Shaders/HelloCompute_Release.hpp"
#endif

#include <Shaders/Shared.hpp>

using namespace DirectX;

class HelloCompute : public Renderer
{
	enum
	{
		Output,
		Count
	};

	void InitializeSample() override;
	void RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList) override;

	void Update(float deltaTime) override;
private:
	
	ComputePipeline* m_ComputePipeline = nullptr;
};

SAMPLE(HelloCompute)

void HelloCompute::InitializeSample()
{
	ComputePipelineDesc desc = {};

	CD3DX12_DESCRIPTOR_RANGE range = {};
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER params[Count] = {};
	params[Output].InitAsDescriptorTable(1, &range);

	desc.RootSignatureDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(params), params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);
	desc.ShaderCode.pShaderBytecode = g_HelloCompute;
	desc.ShaderCode.BytecodeLength = sizeof(g_HelloCompute);

	m_ComputePipeline = new ComputePipeline(desc);
}


void HelloCompute::RenderSample(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> cmdList)
{
	cmdList->SetComputeRootSignature(m_ComputePipeline->GetRootSignature().Get());
	cmdList->SetPipelineState(m_ComputePipeline->GetPipelineState().Get());

	cmdList->SetComputeRootDescriptorTable(Output, m_ShaderHeap->GetGPUHandle(m_UAV));

	cmdList->Dispatch(m_Width / 16, m_Height / 16, 1);
}

void HelloCompute::Update(float deltaTime)
{
}