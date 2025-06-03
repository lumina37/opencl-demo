function(clc_add_sample name src)
    add_executable(${name} ${src})

    target_link_libraries(${name} PRIVATE clc::lib::static clc::shader)

    copy_dlls_if_needed(${name})
endfunction()

clc_add_sample(clc-bin-grayscale grayscale.cpp)
clc_add_sample(clc-bin-grayscale-uma grayscale-uma.cpp)
