#pragma once

#include <CL/cl.h>

#include "clc/device/platform.hpp"

namespace clc {

class DeviceManager {
    DeviceManager(cl_device_id&& device) noexcept;

public:
    [[nodiscard]] static std::expected<DeviceManager, cl_int> create(PlatformManager& platformMgr) noexcept;

    [[nodiscard]] cl_device_id getDevice() const noexcept { return device_; }

private:
    cl_device_id device_;
    cl_uint imagePitchAlign_ ;
    cl_uint imageBaseAddrAlign_;
    bool supportImageFromBuffer_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/device.cpp"
#endif
