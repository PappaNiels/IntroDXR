#pragma once

#include <string>
#include <string_view>

#include <cstdint>

#include <memory>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#include <Windows.h>

#include <d3d12.h>
#include <d3dx12.h>

#include <dxgi1_6.h>

#define UNUSED [[maybe_unused]]