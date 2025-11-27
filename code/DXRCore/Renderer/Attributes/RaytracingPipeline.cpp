#include "pch.hpp"
#include "RaytracingPipeline.hpp"

#include "Device.hpp"

#include <Utils/Error.hpp>
#include <Utils/Assert.hpp>

RaytracingPipeline::RaytracingPipeline(std::string_view, void*)
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
	// we do not support local root signatures (yet). support would mean that the buffer creation needs to be more flexable.

	auto device = Device::GetDevice().GetInternalDevice();

	void* rayGenShaderIdentifier;
	//void* missShaderIdentifier;
	//void* hitGroupShaderIdentifier;

	auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
		{
			rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(desc.RayGenEntry.EntryName.data());
			//missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(desc.MissShaders[0].EntryName.data());
			//hitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(desc.HitGroups[0].HitGroup.data());
		};

	// Get shader identifiers.
	uint32_t shaderIdentifierSize;

	Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProperties;

	auto hr = m_Pipeline.As(&stateObjectProperties);
	if (FAILED(hr))
	{
		FatalError("...");
	}
	GetShaderIdentifiers(stateObjectProperties.Get());
	shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

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

		m_RayGenShaderTable->SetName(L"Ray Generation Shader Table");
	}

	// Miss
	{
		UINT shaderRecordSize = Align(shaderIdentifierSize, shaderRecordAlignment);
		UINT tableSize = shaderRecordSize * static_cast<uint32_t>(desc.MissShaders.size());

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

		uint32_t offset = 0;
		for (const auto& miss : desc.MissShaders)
		{
			auto* missShader = stateObjectProperties->GetShaderIdentifier(miss.EntryName.data());
			memcpy(mappedData + offset * shaderIdentifierSize, missShader, shaderIdentifierSize);

			offset++;
		}

		m_MissShaderTable->Unmap(0, nullptr);
		m_RayGenShaderTable->SetName(L"Miss Shader Table");
	}

	// Hit
	{
		const UINT shaderRecordSize = Align(shaderIdentifierSize, shaderRecordAlignment);
		const UINT tableSize = shaderRecordSize * static_cast<uint32_t>(desc.HitGroups.size());

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

		uint32_t offset = 0;
		for (const auto& hit : desc.HitGroups)
		{
			auto* hitGroup = stateObjectProperties->GetShaderIdentifier(hit.HitGroup.data());
			memcpy(mappedData + offset * shaderIdentifierSize, hitGroup, shaderIdentifierSize);

			offset++;
		}

		m_HitShaderTable->Unmap(0, nullptr);
		m_RayGenShaderTable->SetName(L"Hit Group Shader Table");
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

	for (const auto& hit : desc.HitGroups)
	{
		auto* hitGroup = pipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		
		if (!hit.ClosestHit.empty())
		{
			hitGroup->SetClosestHitShaderImport(hit.ClosestHit.data());
		}

		if (!hit.AnyHit.empty())
		{
			hitGroup->SetAnyHitShaderImport(hit.AnyHit.data());
		}

		if (!hit.Intersection.empty())
		{
			ASSERT(hit.Type == D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE, "Procedural primitives must be used with the intersection shader");

			hitGroup->SetIntersectionShaderImport(hit.Intersection.data());
		}

		hitGroup->SetHitGroupExport(hit.HitGroup.data());
		hitGroup->SetHitGroupType(hit.Type);
	}

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
