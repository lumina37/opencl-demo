#pragma once

#include <format>
#include <iostream>
#include <print>
#include <stdexcept>
#include <string>

#include "clc/helper/defines.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/helper/exception.hpp"
#endif

namespace clc {

void checkError(const cl_int errCode) {
    if (errCode != CL_SUCCESS) {
        if constexpr (ENABLE_DEBUG) {
            auto errStr = std::format("OpenCL err: {}!", errCode);
            std::println(std::cerr, "{}", errStr);
            throw std::runtime_error(errStr);
        }
    }
}

}  // namespace clc
