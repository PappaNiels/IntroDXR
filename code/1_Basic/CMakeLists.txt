set(SAMPLE_NAME RaytracingBasic)

find_program(DXC_EXECUTABLE dxc
    HINTS
        "C:/Program Files (x86)/Windows Kits/10/bin/x64"
    REQUIRED
)

set(SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${SAMPLE_NAME}.hlsl)
set(SHADER_HDR  
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${SAMPLE_NAME}_Debug.hpp>
    $<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${SAMPLE_NAME}_Release.hpp>
)

add_custom_command(
    OUTPUT ${SHADER_HDR}
    COMMAND ${DXC_EXECUTABLE}
        -T lib_6_6
        -HV 2021
        $<$<CONFIG:Debug>:-O0>
        $<$<NOT:$<CONFIG:Debug>>:-O3>
        -Vn g_${SAMPLE_NAME}
        $<$<NOT:$<CONFIG:Debug>>:-Qstrip_debug>
        -Qstrip_reflect
        -Fh ${SHADER_HDR}
        "${SHADER_SRC}"
    DEPENDS ${SHADER_SRC}
    COMMENT "Compiling HLSL ${SHADER_SRC}"
)

add_custom_target(Shaders ALL
    DEPENDS ${SHADER_HDR}
)

add_executable(1_Basic WIN32)
add_dependencies(1_Basic Shaders)

target_link_libraries(1_Basic DXRCore)

target_compile_options(1_Basic PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

set_target_properties(1_Basic PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_precompile_headers(1_Basic PRIVATE pch.hpp)

set_target_properties(1_Basic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..
)

target_include_directories(1_Basic PUBLIC ${CMAKE_SOURCE_DIR}/code)
target_include_directories(1_Basic PUBLIC ${CMAKE_SOURCE_DIR}/code/packages/Microsoft.Direct3D.D3D12.1.618.2/build/native/include)

target_sources(1_Basic PRIVATE
    Basic.cpp
    pch.cpp
)

add_custom_command(
    TARGET 1_Basic
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/code/packages/Microsoft.Direct3D.D3D12.1.618.2/build/native/bin/x64/D3D12Core.dll"
        "$<TARGET_FILE_DIR:1_Basic>/D3D12/D3D12Core.dll"
)

add_custom_command(
    TARGET 1_Basic
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/code/packages/Microsoft.Direct3D.D3D12.1.618.2/build/native/bin/x64/D3D12Core.pdb"
        "$<TARGET_FILE_DIR:1_Basic>/D3D12/D3D12Core.pdb"
)

add_custom_command(
    TARGET 1_Basic
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/code/packages/Microsoft.Direct3D.D3D12.1.618.2/build/native/bin/x64/d3d12SDKLayers.dll"
        "$<TARGET_FILE_DIR:1_Basic>/D3D12/d3d12SDKLayers.dll"
)

add_custom_command(
    TARGET 1_Basic
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/code/packages/Microsoft.Direct3D.D3D12.1.618.2/build/native/bin/x64/d3d12SDKLayers.pdb"
        "$<TARGET_FILE_DIR:1_Basic>/D3D12/d3d12SDKLayers.pdb"
)