#include <filesystem>
#include <format>
#include <source_location>
#include <string>

#include "clc/common/info.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "clc/helper/error.hpp"
#endif

namespace clc {

namespace fs = std::filesystem;

Error::Error(const cl_int clErr, const std::source_location& srcLoc) : clErr(clErr) {
    const fs::path absFilePath{srcLoc.file_name()};
    const fs::path relFilePath = fs::relative(absFilePath, includeBase);
    this->msg = std::format("{}:{}", relFilePath.string(), srcLoc.line(), clErr);
}

Error::Error(const cl_int clErr, const std::string& msg, const std::source_location& srcLoc) : clErr(clErr) {
    const fs::path absFilePath{srcLoc.file_name()};
    const fs::path relFilePath = fs::relative(absFilePath, includeBase);
    this->msg = std::format("{}:{} {}", relFilePath.string(), srcLoc.line(), clErr, msg);
}

}  // namespace clc
