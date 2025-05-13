cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(OpenCLCemo
        VERSION 0.0.1
        DESCRIPTION "OpenCL demo"
        LANGUAGES CXX
)

# Compile Options
option(CLC_VERBOSE_WARNING "Show verbose compiler warnings" OFF)

# Dependencies
include(FetchContent)

find_package(OpenCL REQUIRED)

FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# Helper
function(copy_dlls_if_needed name)
    if (WIN32)
        add_custom_command(TARGET ${name} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
                copy_if_different $<TARGET_RUNTIME_DLLS:${name}> $<TARGET_FILE:${name}> $<TARGET_FILE_DIR:${name}> COMMAND_EXPAND_LISTS)
    endif ()
endfunction()

# Sources
add_subdirectory(src)
add_subdirectory(kernel)
add_subdirectory(bin)
