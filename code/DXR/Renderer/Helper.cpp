#include "pch.hpp"
#include "Helper.hpp"

#include "Attributes/Device.hpp"

CommandList CreateCommandList(D3D12_COMMAND_LIST_TYPE type)
{
    auto device = Device::GetDevice().GetInternalDevice();

    CommandList cmdList = {};

    device->CreateCommandAllocator(type, IID_PPV_ARGS(&cmdList.CommandAllocator));
    device->CreateCommandList(0, type, cmdList.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList.CommandList));

    return cmdList;
}
