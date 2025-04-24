#include <filesystem>
#include <string>

#include "clc/common/config.h"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "clc/common/info.hpp"
#endif

namespace clc {

namespace fs = std::filesystem;

const fs::path includeBase{CLC_INCLUDE_BASE};

}  // namespace clc
