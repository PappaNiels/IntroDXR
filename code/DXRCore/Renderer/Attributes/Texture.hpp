#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <string_view>

class Texture
{
public:
	Texture(const std::string_view path);

	uint32_t GetSRV() const
	{
		return m_SRV;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;

	uint32_t m_SRV;

	bool m_IsHDR;
};

