#pragma once

#include <CL/cl.h>

namespace clc {

class PlatformManager {
public:
    PlatformManager();

    [[nodiscard]] cl_platform_id getPlatform() const noexcept { return platform_; }

private:
    cl_platform_id platform_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/platform.cpp"
#endif
