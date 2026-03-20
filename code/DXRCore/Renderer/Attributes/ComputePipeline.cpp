#include <pch.hpp>

#include "ComputePipeline.hpp"
#include "Device.hpp"

#include <Utils/Error.hpp>

ComputePipeline::ComputePipeline(const ComputePipelineDesc& desc)
{
	CreateRootsignature(desc);
	CreatePipeline(desc);
}

void ComputePipeline::CreateRootsignature(const ComputePipelineDesc& desc)
{
	auto device = Device::GetDevice().GetInternalDevice();

	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	Microsoft::WRL::ComPtr<ID3DBlob> error;

	auto hr = D3D12SerializeRootSignature(&desc.RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);

	if (FAILED(hr))
	{
		if (error != nullptr)
		{
			FatalError("Failed to serialize the global root signature.\nError returned: %s", error->GetBufferPointer());
		}
		else
		{
			FatalError("Failed to serialize the global root signature.");
		}
	}

	hr = device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

	if (FAILED(hr))
	{
		FatalError("Failed to create a root signature");
	}
}

void ComputePipeline::CreatePipeline(const ComputePipelineDesc& desc)
{
	auto device = Device::GetDevice().GetInternalDevice();

	struct PipelineStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_CS CS;
	} stream;

	stream.RootSignature = m_RootSignature.Get();
	stream.CS = desc.ShaderCode;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineDesc = {
		sizeof(PipelineStream), &stream
	};

	auto hr = device->CreatePipelineState(&pipelineDesc, IID_PPV_ARGS(&m_PipelineState));

	if (FAILED(hr))
	{
		FatalError("Failed to create a compute pipeline");
	}
}