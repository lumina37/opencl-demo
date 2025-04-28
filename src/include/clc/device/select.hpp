#pragma once

#include <expected>
#include <print>
#include <utility>

#include <CL/cl.h>

#include "clc/common/defines.hpp"
#include "clc/device/helper.hpp"
#include "clc/device/props.hpp"
#include "clc/device/score.hpp"
#include "clc/helper/error.hpp"

namespace clc {

std::expected<float, Error> defaultJudge(const DeviceWithProps& deviceWithProps) noexcept;

template <CDeviceProps TProps_ = DeviceProps>
class Devices_ {
public:
    using TProps = TProps_;
    using TDeviceWithProps = DeviceWithProps_<TProps>;
    using FnJudge = std::expected<float, Error> (*)(const TDeviceWithProps&) noexcept;

private:
    Devices_(std::vector<TDeviceWithProps>&& deviceWithPropsVec) noexcept;

public:
    [[nodiscard]] static std::expected<Devices_, Error> create() noexcept;

    [[nodiscard]] std::expected<std::reference_wrapper<TDeviceWithProps>, Error> select(
        const FnJudge& judge = defaultJudge) noexcept;

private:
    std::vector<TDeviceWithProps> deviceWithPropsVec_;
};

template <CDeviceProps TProps>
Devices_<TProps>::Devices_(std::vector<TDeviceWithProps>&& deviceWithPropsVec) noexcept
    : deviceWithPropsVec_(std::move(deviceWithPropsVec)) {}

template <CDeviceProps TProps>
std::expected<Devices_<TProps>, Error> Devices_<TProps>::create() noexcept {
    auto platformsRes = getPlatformIDs();
    if (!platformsRes) return std::unexpected{std::move(platformsRes.error())};
    const auto& platforms = platformsRes.value();

    std::vector<TDeviceWithProps> deviceWithPropsVec;
    for (const auto& platform : platforms) {
        auto devicesRes = getDeviceIDs(platform);
        if (!devicesRes) return std::unexpected{std::move(devicesRes.error())};

        const auto& devices = devicesRes.value();
        for (const auto& device : devices) {
            auto deviceMgrRes = DeviceManager::create(platform, device);
            if (!deviceMgrRes) return std::unexpected{std::move(deviceMgrRes.error())};
            auto& deviceMgr = deviceMgrRes.value();

            auto devicePropsRes = DeviceProps::create(device);
            if (!devicePropsRes) return std::unexpected{std::move(devicePropsRes.error())};
            auto& deviceProps = devicePropsRes.value();

            deviceWithPropsVec.emplace_back(std::move(deviceMgr), std::move(deviceProps));
        }
    }

    return Devices_{std::move(deviceWithPropsVec)};
}

template <CDeviceProps TProps>
std::expected<std::reference_wrapper<DeviceWithProps_<TProps>>, Error> Devices_<TProps>::select(
    const FnJudge& judge) noexcept {
    std::vector<Score<std::reference_wrapper<TDeviceWithProps>>> scores;
    scores.reserve(deviceWithPropsVec_.size());

    const auto printDeviceInfo = [](const TDeviceWithProps& deviceWithProps,
                                    const float score) -> std::expected<void, Error> {
        const auto rstrip = [](std::string_view str) {
            size_t lastCh = str.find_last_not_of(' ');
            return str.substr(0, lastCh + 1);
        };

        const cl_device_id device = deviceWithProps.getManager().getDevice();
        const TProps& props = deviceWithProps.getProps();

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

    for (auto& deviceWithProps : deviceWithPropsVec_) {
        auto scoreRes = judge(deviceWithProps);
        if (!scoreRes) return std::unexpected{std::move(scoreRes.error())};

        if constexpr (ENABLE_DEBUG) {
            auto printRes = printDeviceInfo(deviceWithProps, scoreRes.value());
            if (!printRes) return std::unexpected{std::move(printRes.error())};
        }

        scores.emplace_back(scoreRes.value(), std::ref(deviceWithProps));
    }

    if (scores.empty()) {
        return std::unexpected{1};
    }

    auto maxScoreIt = std::max_element(scores.begin(), scores.end());
    return std::move(maxScoreIt->attachment);
}

using Devices = Devices_<>;

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/select.cpp"
#endif
