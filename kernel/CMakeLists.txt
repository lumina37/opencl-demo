set(_CLC_OPENCL_SHADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/opencl/grayscale.cl.h
)

add_custom_target(_clc-shader DEPENDS ${_CLC_OPENCL_SHADERS})
add_library(clc-shader INTERFACE)
add_dependencies(clc-shader _clc-shader)
target_include_directories(clc-shader INTERFACE .)
