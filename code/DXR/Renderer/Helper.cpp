#include "pch.hpp"
#include "Helper.hpp"

#include "Attributes/Device.hpp"

CommandList CreateCommandList(D3D12_COMMAND_LIST_TYPE type)
{
    auto device = Device::GetDevice().GetInternalDevice();

    CommandList cmdList = {};

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdList.CommandAllocator));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdList.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList.CommandList));

    return cmdList;
}
