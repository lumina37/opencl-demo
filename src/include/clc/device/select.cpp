#include <expected>
#include <print>
#include <utility>

#include <CL/cl.h>

#include "clc/common/defines.hpp"
#include "clc/device/helper.hpp"
#include "clc/device/props.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/select.hpp"
#endif

namespace clc {

std::expected<float, Error> defaultJudge(const DeviceWithProps& deviceWithProps) noexcept {
    const auto getDeviceScore = [](const cl_device_id device, const DeviceProps& props) -> std::expected<float, Error> {
        if (props.deviceVersionMajor < 2) return std::numeric_limits<float>::lowest();

        auto imageSupportRes = getDeviceInfo<cl_bool>(device, CL_DEVICE_IMAGE_SUPPORT);
        if (!imageSupportRes) return std::unexpected{std::move(imageSupportRes.error())};
        if (imageSupportRes.value() == CL_FALSE) return std::numeric_limits<float>::lowest();

        float score = 0;
        if (props.realLocalMem) {
            score = (float)props.localMemSize;
        }
        if (props.deviceType & CL_DEVICE_TYPE_GPU) score *= 2;

        return score;
    };

    const auto printDeviceInfo = [](const cl_device_id device, const DeviceProps& props,
                                    const float score) -> std::expected<void, Error> {
        const auto rstrip = [](std::string_view str) {
            size_t lastCh = str.find_last_not_of(' ');
            return str.substr(0, lastCh + 1);
        };

        auto deviceNameRes = getDeviceInfo<char[]>(device, CL_DEVICE_NAME);
        if (!deviceNameRes) return std::unexpected{std::move(deviceNameRes.error())};
        const auto deviceName = rstrip(std::string_view{deviceNameRes.value().data()});

        const auto deviceVersionRes = getDeviceInfo<char[]>(device, CL_DEVICE_VERSION);
        if (!deviceVersionRes) return std::unexpected{std::move(deviceVersionRes.error())};
        const auto deviceVersion = rstrip(std::string_view{deviceVersionRes.value().data()});

        std::println("Candidate device: name={}, ver={} ({}.{}), type={}, score={}", deviceName, deviceVersion,
                     props.deviceVersionMajor, props.deviceVersionMinor, props.deviceType, score);
        std::println("Extensions: {}", props.extensions);

        return {};
    };

    const cl_device_id device = deviceWithProps.getManager().getDevice();
    const DeviceProps& props = deviceWithProps.getProps();

    auto scoreRes = getDeviceScore(device, props);
    if (!scoreRes) return std::unexpected{std::move(scoreRes.error())};

    if constexpr (ENABLE_DEBUG) {
        auto printRes = printDeviceInfo(device, props, scoreRes.value());
        if (!printRes) return std::unexpected{std::move(printRes.error())};
    }

    return scoreRes.value();
}

template class Devices_<DeviceProps>;

}  // namespace clc
