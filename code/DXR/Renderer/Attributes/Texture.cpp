#include "pch.hpp"
#include "Texture.hpp"

#include "Device.hpp"
#include "DescriptorHeap.hpp"

#include <stb_image.h>

#include <Utils/Assert.hpp>
#include <Utils/Error.hpp>

#include <Renderer/Helper.hpp>
#include <Renderer/Renderer.hpp>

Texture::Texture(const std::string_view path)
	: m_SRV(static_cast<uint32_t>(-1))
	, m_IsHDR(false)
{
	ASSERT(path.find("../") != static_cast<size_t>(-1), "'../' was missing from the texture path. Add it to make sure it points to the correct directory, so it can read the file.");

	void* data = nullptr;
	m_IsHDR = path.find(".hdr") != static_cast<size_t>(-1);

	int32_t width;
	int32_t height;
	int32_t nrChannels;

	if (m_IsHDR)
	{
		data = stbi_loadf(path.data(), &width, &height, &nrChannels, 4);
	}
	else
	{
		data = stbi_load(path.data(), &width, &height, &nrChannels, 4);
	}

	if (data == nullptr)
	{
		char workingDir[128];
		GetCurrentDirectoryA(128, workingDir);

		FatalError("Failed to load texture data.\n\nFile path: %s.\nWorking directory: %s.\nstbi failure reason: %s", path.data(), workingDir, stbi_failure_reason());
	}

	auto device = Device::GetDevice().GetInternalDevice();
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_IsHDR ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1);
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	ASSERT(SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Resource))), "Failed to create image resource");

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediate;

	auto size = GetRequiredIntermediateSize(m_Resource.Get(), 0, 1);
	resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	ASSERT(SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&intermediate))), "Failed to create image resource");

	auto cmdList = CreateCommandList();
	auto& cmdQueue = Device::GetDevice().GetCommandQueue();

	D3D12_SUBRESOURCE_DATA resData = {};
	resData.pData = data;
	resData.RowPitch = width * (m_IsHDR ? 4 * sizeof(float) : 4 * sizeof(uint8_t));
	resData.SlicePitch = resData.RowPitch * height;

	UpdateSubresources(cmdList.CommandList.Get(), m_Resource.Get(), intermediate.Get(), 0, 0, 1, &resData);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdList.CommandList->ResourceBarrier(1, &barrier);

	auto fence = cmdQueue.ExecuteCommandLists({ cmdList.CommandList.Get() });
	cmdQueue.WaitForFence(fence);

	auto* heap = Renderer::GetShaderHeap();
	m_SRV = heap->GetNextIndex();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Format = m_IsHDR ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	device->CreateShaderResourceView(m_Resource.Get(), &desc, heap->GetCPUHandle(m_SRV));

	stbi_image_free(data);
}
