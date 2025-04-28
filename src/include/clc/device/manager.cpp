#include <expected>
#include <utility>

#include <CL/cl.h>

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/manager.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

DeviceManager::DeviceManager(cl_platform_id&& platform, cl_device_id&& device) noexcept
    : platform_(platform), device_(device) {}

std::expected<DeviceManager, Error> DeviceManager::create(cl_platform_id platform, cl_device_id device) noexcept {
    return DeviceManager{std::move(platform), std::move(device)};
}

}  // namespace clc
