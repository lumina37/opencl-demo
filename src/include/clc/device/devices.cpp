#include <expected>
#include <limits>

#include <CL/cl.h>

#include "clc/device/props.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/devices.hpp"
#endif

namespace clc {

std::expected<float, Error> defaultJudge(const DeviceWithProps_<DeviceProps>& deviceWithProps) noexcept {
    const DeviceProps& props = deviceWithProps.getProps();

    if (props.deviceVersionMajor < 2) return std::numeric_limits<float>::lowest();
    if (!props.supportImage) return std::numeric_limits<float>::lowest();

    float score = 0;
    if (props.realLocalMem) {
        score = (float)props.localMemSize;
    }
    if (props.deviceType & CL_DEVICE_TYPE_GPU) score *= 2;

    return score;
}

template class Devices_<DeviceProps>;

}  // namespace clc
