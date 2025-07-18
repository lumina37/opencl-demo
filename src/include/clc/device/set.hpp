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

template <CDeviceProps TProps_>
class DeviceSet_ {
public:
    using TProps = TProps_;
    using TDeviceWithProps = DeviceWithProps_<TProps>;
    using FnJudge = std::expected<float, Error> (*)(const TDeviceWithProps&) noexcept;

private:
    DeviceSet_(std::vector<TDeviceWithProps>&& devicesWithProps) noexcept;

public:
    DeviceSet_(const DeviceSet_&) noexcept = delete;
    DeviceSet_& operator=(const DeviceSet_&) noexcept = delete;
    DeviceSet_(DeviceSet_&& rhs) noexcept = default;
    DeviceSet_& operator=(DeviceSet_&& rhs) noexcept = default;

    [[nodiscard]] static std::expected<DeviceSet_, Error> create() noexcept;

    [[nodiscard]] std::expected<std::reference_wrapper<TDeviceWithProps>, Error> select(const FnJudge& judge) noexcept;
    [[nodiscard]] std::expected<std::reference_wrapper<TDeviceWithProps>, Error> selectDefault() noexcept;

private:
    std::vector<TDeviceWithProps> devicesWithProps_;
};

template <CDeviceProps TProps>
DeviceSet_<TProps>::DeviceSet_(std::vector<TDeviceWithProps>&& devicesWithProps) noexcept
    : devicesWithProps_(std::move(devicesWithProps)) {}

template <CDeviceProps TProps>
std::expected<DeviceSet_<TProps>, Error> DeviceSet_<TProps>::create() noexcept {
    auto platformsRes = getPlatformIDs();
    if (!platformsRes) return std::unexpected{std::move(platformsRes.error())};
    const auto& platforms = platformsRes.value();

    std::vector<TDeviceWithProps> devicesWithProps;
    for (const auto& platform : platforms) {
        auto devicesRes = getDeviceIDs(platform);
        if (!devicesRes) return std::unexpected{std::move(devicesRes.error())};

        const auto& devices = devicesRes.value();
        for (const auto& device : devices) {
            auto deviceBoxRes = DeviceBox::create(platform, device);
            if (!deviceBoxRes) return std::unexpected{std::move(deviceBoxRes.error())};
            auto& deviceBox = deviceBoxRes.value();

            auto devicePropsRes = TProps::create(device);
            if (!devicePropsRes) return std::unexpected{std::move(devicePropsRes.error())};
            auto& deviceProps = devicePropsRes.value();

            devicesWithProps.emplace_back(std::move(deviceBox), std::move(deviceProps));
        }
    }

    return DeviceSet_{std::move(devicesWithProps)};
}

template <CDeviceProps TProps>
auto DeviceSet_<TProps>::select(const FnJudge& judge) noexcept
    -> std::expected<std::reference_wrapper<TDeviceWithProps>, Error> {
    std::vector<Score<std::reference_wrapper<TDeviceWithProps>>> scores;
    scores.reserve(devicesWithProps_.size());

    const auto printDeviceInfo = [](const TDeviceWithProps& deviceWithProps,
                                    const float score) -> std::expected<void, Error> {
        const auto rstrip = [](std::string_view str) {
            size_t lastCh = str.find_last_not_of(' ');
            return str.substr(0, lastCh + 1);
        };

        const cl_device_id device = deviceWithProps.getDeviceBox().getDevice();
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

    for (auto& deviceWithProps : devicesWithProps_) {
        auto scoreRes = judge(deviceWithProps);
        if (!scoreRes) return std::unexpected{std::move(scoreRes.error())};

        if constexpr (ENABLE_DEBUG) {
            auto printRes = printDeviceInfo(deviceWithProps, scoreRes.value());
            if (!printRes) return std::unexpected{std::move(printRes.error())};
        }

        scores.emplace_back(scoreRes.value(), std::ref(deviceWithProps));
    }

    if (scores.empty()) {
        return std::unexpected{Error{ECate::eCLC, ECode::eNoSupport, "no sufficient device"}};
    }

    auto maxScoreIt = std::max_element(scores.begin(), scores.end());
    return std::move(maxScoreIt->attachment);
}

template <CDeviceProps TProps>
auto DeviceSet_<TProps>::selectDefault() noexcept -> std::expected<std::reference_wrapper<TDeviceWithProps>, Error> {
    constexpr auto defaultJudge = [](const TDeviceWithProps& deviceWithProps) noexcept {
        return deviceWithProps.getProps().score();
    };
    return select(defaultJudge);
}

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/set.cpp"
#endif
