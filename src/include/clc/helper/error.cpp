#include <source_location>
#include <string>
#include <utility>

#include "clc/common/info.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "clc/helper/error.hpp"
#endif

namespace clc {

Error::Error(const int code, const std::source_location& source) : code(code), source(source) {}

Error::Error(const int code, const std::string& msg, const std::source_location& source)
    : code(code), source(source), msg(msg) {}

Error::Error(const int code, std::string&& msg, const std::source_location& source)
    : code(code), source(source), msg(std::move(msg)) {}

}  // namespace clc
