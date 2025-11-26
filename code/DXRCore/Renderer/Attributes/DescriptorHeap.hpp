#pragma once

enum class HeapType
{
	RTV,
	Shader
};

class DescriptorHeap
{
public:
	void Initialize(HeapType type);

	uint32_t GetNextIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap() const
	{
		return m_Heap;
	}
protected:
	friend class Renderer;
	DescriptorHeap() = default;
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;

	uint32_t m_IncreaseSize = 0;
	uint32_t m_CurrentIndex = 0;
	uint32_t m_MaxIndex = 32;
};

