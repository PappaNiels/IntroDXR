#include "pch.hpp"
#include "RaytracingPipeline.hpp"

#include "Device.hpp"

#include <Utils/Error.hpp>
#include <Utils/Assert.hpp>

RaytracingPipeline::RaytracingPipeline(std::string_view name, void* code)
{
	/*CreateGlobalRootSignature();
	CreatePipeline();
	CreateShaderTables();*/
}

RaytracingPipeline::RaytracingPipeline(const RaytracingPipelineDesc& desc)
{
	CreateGlobalRootSignature(desc);
	CreatePipeline(desc);
	CreateShaderTables(desc);
}

void RaytracingPipeline::CreateGlobalRootSignature(const RaytracingPipelineDesc& desc)
{
	auto device = Device::GetDevice().GetInternalDevice();

	CD3DX12_ROOT_PARAMETER params[2] = {};
	params[0].InitAsConstants(17, 0);
	params[1].InitAsShaderResourceView(0);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(params), params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);

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

void RaytracingPipeline::CreateShaderTables(const RaytracingPipelineDesc& desc)
{
	auto device = Device::GetDevice().GetInternalDevice();

	void* rayGenShaderIdentifier;
	void* missShaderIdentifier;
	void* hitGroupShaderIdentifier;

	auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
		{
			rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(desc.RayGenEntry.EntryName.data());
			missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(desc.MissShaders[0].EntryName.data());
			hitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(desc.HitGroups[0].HitGroup.data());
		};

	// Get shader identifiers.
	uint32_t shaderIdentifierSize;
	{
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
		
		auto hr = m_Pipeline.As(&stateObjectProperties);
		if (FAILED(hr))
		{
			FatalError("...");
		}
		GetShaderIdentifiers(stateObjectProperties.Get());
		shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	}

	const uint32_t shaderRecordAlignment = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

	auto Align = [](UINT size, UINT alignment) {
		return (size + (alignment - 1)) & ~(alignment - 1);
		};

	// Ray Gen 
	{
		UINT shaderRecordSize = Align(shaderIdentifierSize, shaderRecordAlignment);
		UINT tableSize = shaderRecordSize;

		// Create buffer
		CD3DX12_RESOURCE_DESC descShader = CD3DX12_RESOURCE_DESC::Buffer(tableSize, D3D12_RESOURCE_FLAG_NONE);
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

		device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&descShader,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_RayGenShaderTable)
		);

		// Map and copy data
		uint8_t* mappedData = nullptr;
		m_RayGenShaderTable->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
		memcpy(mappedData, rayGenShaderIdentifier, shaderIdentifierSize);
		m_RayGenShaderTable->Unmap(0, nullptr);
	}

	// Miss
	{
		UINT shaderRecordSize = Align(shaderIdentifierSize, shaderRecordAlignment);
		UINT tableSize = shaderRecordSize;

		CD3DX12_RESOURCE_DESC descShader = CD3DX12_RESOURCE_DESC::Buffer(tableSize, D3D12_RESOURCE_FLAG_NONE);
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

		device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&descShader,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_MissShaderTable)
		);

		uint8_t* mappedData = nullptr;
		m_MissShaderTable->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
		memcpy(mappedData, missShaderIdentifier, shaderIdentifierSize);
		m_MissShaderTable->Unmap(0, nullptr);
	}

	// Hit
	{
		const UINT shaderRecordSize = Align(shaderIdentifierSize, shaderRecordAlignment);
		const UINT tableSize = Align(shaderRecordSize, shaderRecordAlignment);

		CD3DX12_RESOURCE_DESC descShader = CD3DX12_RESOURCE_DESC::Buffer(tableSize, D3D12_RESOURCE_FLAG_NONE);
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

		device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&descShader,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_HitShaderTable)
		);

		uint8_t* mappedData = nullptr;
		m_HitShaderTable->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
		memcpy(mappedData, hitGroupShaderIdentifier, shaderIdentifierSize);
		m_HitShaderTable->Unmap(0, nullptr);
	}
}

void RaytracingPipeline::CreatePipeline(const RaytracingPipelineDesc& desc)
{
	auto device = Device::GetDevice().GetInternalDevice();

	CD3DX12_STATE_OBJECT_DESC pipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

	auto* library = pipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	library->SetDXILLibrary(&desc.ShaderCode);

	// Define function entries
	/*library->DefineExport(desc.RayGenEntry.EntryName.data());
	library->DefineExport(desc.HitGroups[0].ShaderEntry.data());
	library->DefineExport(desc.MissShaders[0].EntryName.data());*/

	auto* hitGroup = pipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
	hitGroup->SetClosestHitShaderImport(desc.HitGroups[0].ShaderEntry.data());
	hitGroup->SetHitGroupExport(desc.HitGroups[0].HitGroup.data());
	hitGroup->SetHitGroupType(desc.HitGroups[0].Type);

	auto* shaderConfig = pipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	shaderConfig->Config(desc.PayloadSize, desc.AttributeSize);

	// create local root signatures

	auto* globalRS = pipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRS->SetRootSignature(m_RootSignature.Get());

	auto pipelineConfig = pipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	pipelineConfig->Config(desc.RecursionDepth);

	auto hr = device->CreateStateObject(pipeline, IID_PPV_ARGS(&m_Pipeline));

	if (FAILED(hr))
	{
		FatalError("Failed to create a ray tracing state object. HResult: 0x%08X", hr);
	}
}
