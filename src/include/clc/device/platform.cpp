#include <memory>
#include <utility>

#include <CL/cl.h>

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/platform.hpp"
#endif

namespace clc {

PlatformManager::PlatformManager(cl_platform_id&& platform) noexcept : platform_(platform) {}

std::expected<PlatformManager, cl_int> PlatformManager::create() noexcept {
    cl_int clErr;

    cl_uint platformCount;
    clErr = clGetPlatformIDs(0, nullptr, &platformCount);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    auto pPlatforms = std::make_unique_for_overwrite<cl_platform_id[]>(platformCount);
    clErr = clGetPlatformIDs(platformCount, pPlatforms.get(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    cl_platform_id platform = pPlatforms[0];
    return PlatformManager{std::move(platform)};
}

}  // namespace clc
