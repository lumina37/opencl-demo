#include <memory>

#include <CL/cl.h>

#include "clc/helper/exception.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/platform.hpp"
#endif

namespace clc {

PlatformManager::PlatformManager() {
    cl_int errCode;

    cl_uint platformCount;
    errCode = clGetPlatformIDs(0, nullptr, &platformCount);
    checkError(errCode);

    auto pPlatforms = std::make_unique_for_overwrite<cl_platform_id[]>(platformCount);
    errCode = clGetPlatformIDs(platformCount, pPlatforms.get(), nullptr);
    checkError(errCode);

    platform_ = pPlatforms[0];
}

}  // namespace clc
