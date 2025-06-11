#pragma once

#include <source_location>
#include <string>

namespace clc {

class Error {
public:
    int code;
    std::source_location source;
    std::string msg;

    Error() = default;
    explicit Error(int code, const std::source_location& source = std::source_location::current());
    Error(int code, const std::string& msg, const std::source_location& source = std::source_location::current());
    Error(int code, std::string&& msg, const std::source_location& source = std::source_location::current());
    Error(const Error& rhs) = default;
    Error& operator=(const Error& rhs) = default;
    Error(Error&& rhs) noexcept = default;
    Error& operator=(Error&& rhs) = default;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/helper/error.cpp"
#endif
