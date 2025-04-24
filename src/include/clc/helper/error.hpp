#pragma once

#include <source_location>
#include <string>

#include <CL/cl.h>

namespace clc {

class Error {
public:
    cl_int clErr;
    std::string msg;

    Error(cl_int clErr, const std::source_location& srcLoc = std::source_location::current());
    Error(cl_int clErr, const std::string& msg, const std::source_location& srcLoc = std::source_location::current());
    Error(const Error& rhs) = default;
    Error(Error&& rhs) noexcept = default;
};

}  // namespace clc

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "clc/helper/error.cpp"
#endif
