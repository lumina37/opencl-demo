#pragma once

#include <filesystem>

namespace clc {

namespace fs = std::filesystem;

extern const fs::path includeBase;

}  // namespace clc

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "clc/common/info.cpp"
#endif
