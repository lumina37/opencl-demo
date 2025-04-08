function(clc_add_executable name src)
    add_executable(${name} ${src})

    target_link_libraries(${name} PRIVATE clc-lib clc-shader)

    copy_dlls_if_needed(${name})
endfunction()

clc_add_executable(clc-bin-gaussFilter-v0-baseline main-gaussFilter-v0-baseline.cpp)
