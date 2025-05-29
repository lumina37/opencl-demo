set(_CLC_OPENCL_KERNELS
        grayscale/fp32
        grayscale/fp16
)
set(_CLC_OPENCL_HEADERS)

function(clc_add_kernel name)
    set(__CLC_IN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/opencl/${name}.cl)
    set(__CLC_OUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/opencl/${name}.h)
    list(APPEND _CLC_OPENCL_HEADERS ${__CLC_OUT_PATH})
    set(_CLC_OPENCL_HEADERS ${_CLC_OPENCL_HEADERS} PARENT_SCOPE)

    add_custom_command(
            OUTPUT ${__CLC_OUT_PATH}
            COMMAND ${CMAKE_COMMAND} -DCLC_IN=${__CLC_IN_PATH} -DCLC_OUT=${__CLC_OUT_PATH} -P ${CMAKE_CURRENT_SOURCE_DIR}/cl2h.cmake
            MAIN_DEPENDENCY ${__CLC_IN_PATH}
            COMMENT "Convert OpenCL kernel to header - ${__CLC_IN_PATH}"
    )
endfunction()

foreach (__CLC_OPENCL_KERNEL ${_CLC_OPENCL_KERNELS})
    clc_add_kernel(${__CLC_OPENCL_KERNEL})
endforeach ()

add_custom_target(_clc-shader DEPENDS ${_CLC_OPENCL_HEADERS})

file(GLOB_RECURSE _CLC_KERNEL_SRCS "*.cpp")

add_library(clc-shader STATIC ${_CLC_KERNEL_SRCS})
add_dependencies(clc-shader _clc-shader)
target_compile_features(clc-shader PUBLIC cxx_std_23)
target_include_directories(clc-shader PUBLIC .)
add_library(clc::shader ALIAS clc-shader)
