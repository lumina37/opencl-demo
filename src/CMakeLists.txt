file(GLOB_RECURSE _CLC_SRCS "include/*.cpp")
add_library(clc-lib STATIC ${_CLC_SRCS})

target_compile_features(clc-lib PUBLIC cxx_std_23)
target_compile_definitions(clc-lib PUBLIC CL_TARGET_OPENCL_VERSION=300)
target_link_libraries(clc-lib PUBLIC OpenCL::OpenCL)
target_include_directories(clc-lib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${stb_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(clc-lib PRIVATE ${stb_SOURCE_DIR})
