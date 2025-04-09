function(clc_add_executable name src)
    add_executable(${name} ${src})

    target_link_libraries(${name} PRIVATE clc-lib clc-shader)

    copy_dlls_if_needed(${name})
endfunction()

clc_add_executable(clc-bin-grayscale main-grayscale.cpp)
clc_add_executable(clc-bin-grayscale-uma main-grayscale-uma.cpp)
