add_library(DXRCore STATIC)

target_precompile_headers(DXRCore PRIVATE pch.hpp)

target_compile_options(DXRCore PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

set_target_properties(DXRCore PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

set_target_properties(DXRCore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..
)

target_include_directories(DXRCore PRIVATE ${CMAKE_SOURCE_DIR}/code)
target_include_directories(DXRCore PRIVATE ${CMAKE_SOURCE_DIR}/code/DXRCore)
target_include_directories(DXRCore PUBLIC ${CMAKE_SOURCE_DIR}/code/packages/Microsoft.Direct3D.D3D12.1.618.2/build/native/include)
target_include_directories(DXRCore PUBLIC ${CMAKE_SOURCE_DIR}/code/packages/Microsoft.Direct3D.D3D12.1.618.2/build/native/include/d3dx12)
target_include_directories(DXRCore PRIVATE ${CMAKE_SOURCE_DIR}/vendor/stb)

target_link_libraries(DXRCore PUBLIC d3d12) 
target_link_libraries(DXRCore PUBLIC dxgi)
target_link_libraries(DXRCore PUBLIC dxguid)

target_sources(DXRCore PRIVATE 
    Main.cpp
    pch.cpp
    Renderer\\Helper.cpp
    Renderer\\Renderer.cpp
    Renderer\\Attributes\\CommandQueue.cpp
    Renderer\\Attributes\\DescriptorHeap.cpp
    Renderer\\Attributes\\Device.cpp
    Renderer\\Attributes\\Mesh.cpp
    Renderer\\Attributes\\ProceduralPrimitive.cpp
    Renderer\\Attributes\\RaytracingPipeline.cpp
    Renderer\\Attributes\\SwapChain.cpp
    Renderer\\Attributes\\Texture.cpp
    Renderer\\Attributes\\TLAS.cpp
    Utils\\Assert.cpp
    Utils\\CLI.cpp
    Utils\\Error.cpp
)

find_program(NUGET nuget)

if (NOT NUGET)
    message(FATAL " Nuget was not found. Installation is needed")
else()
    execute_process(COMMAND ${NUGET} restore ${CMAKE_CURRENT_SOURCE_DIR}/packages.config -SolutionDir ${CMAKE_CURRENT_SOURCE_DIR}/../)
endif() 
