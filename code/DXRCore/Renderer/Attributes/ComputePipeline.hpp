#pragma once

struct ComputePipelineDesc
{
	D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc;
	D3D12_SHADER_BYTECODE ShaderCode;
};

class ComputePipeline
{
public:
	ComputePipeline(const ComputePipelineDesc& desc);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const
	{
		return m_RootSignature;
	}

	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState() const
	{
		return m_PipelineState;
	}

private:
	void CreateRootsignature(const ComputePipelineDesc& desc);
	void CreatePipeline(const ComputePipelineDesc& desc);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
};