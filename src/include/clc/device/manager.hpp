#pragma once

#include <expected>

#include <CL/cl.h>

#include "clc/helper/error.hpp"

namespace clc {

class DeviceManager {
    DeviceManager(cl_platform_id platform, cl_device_id device) noexcept;

public:
    [[nodiscard]] static std::expected<DeviceManager, Error> create(cl_platform_id platform,
                                                                    cl_device_id device) noexcept;

    [[nodiscard]] cl_device_id getDevice() const noexcept { return device_; }

private:
    cl_platform_id platform_;
    cl_device_id device_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/manager.cpp"
#endif
