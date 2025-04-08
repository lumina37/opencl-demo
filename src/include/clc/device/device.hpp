#pragma once

#include <CL/cl.h>

#include "clc/device/platform.hpp"

namespace clc {

class DeviceManager {
public:
    DeviceManager(PlatformManager& platformMgr);

    [[nodiscard]] cl_device_id getDevice() const noexcept { return device_; }

private:
    cl_device_id device_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/device.cpp"
#endif
