function(clc_common_lib_setting name)
    get_target_property(__TYPE ${name} TYPE)
    if (${__TYPE} STREQUAL "INTERFACE_LIBRARY")
        set(__PUB_DEP_SCOPE "INTERFACE")
    else ()
        set(__PUB_DEP_SCOPE "PUBLIC")
    endif ()

    target_compile_features(${name} ${__PUB_DEP_SCOPE} cxx_std_23)
    target_compile_definitions(${name} ${__PUB_DEP_SCOPE} CL_TARGET_OPENCL_VERSION=300)

    target_include_directories(${name} ${__PUB_DEP_SCOPE}
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${stb_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_link_libraries(${name} ${__PUB_DEP_SCOPE} OpenCL::OpenCL)

    if (CLC_VERBOSE_WARNING)
        if (MSVC)
            target_compile_options(${name} ${__PUB_DEP_SCOPE} /W4)
        else ()
            target_compile_options(${name} ${__PUB_DEP_SCOPE} -Wall -Wextra)
        endif ()
    endif ()
endfunction()

file(GLOB_RECURSE _CLC_SRCS "include/*.cpp")

add_library(clc-lib-static STATIC ${_CLC_SRCS})
clc_common_lib_setting(clc-lib-static)
add_library(clc::lib::static ALIAS clc-lib-static)

add_library(clc-lib-header-only INTERFACE)
clc_common_lib_setting(clc-lib-header-only)
target_compile_definitions(clc-lib-header-only INTERFACE "_CLC_LIB_HEADER_ONLY")
add_library(clc::lib::header-only ALIAS clc-lib-header-only)
