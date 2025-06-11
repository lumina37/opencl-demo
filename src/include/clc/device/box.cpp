#include <expected>
#include <utility>

#include <CL/cl.h>

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/box.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

DeviceBox::DeviceBox(cl_platform_id platform, cl_device_id device) noexcept : platform_(platform), device_(device) {}

std::expected<DeviceBox, Error> DeviceBox::create(cl_platform_id platform, cl_device_id device) noexcept {
    return DeviceBox{platform, device};
}

}  // namespace clc
