#include <algorithm>
#include <expected>
#include <print>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <CL/cl.h>

#include "clc/common/defines.hpp"
#include "clc/device/props.hpp"
#include "clc/device/score.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/device.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

DeviceManager::DeviceManager(cl_platform_id&& platform, cl_device_id&& device, DeviceProps&& props) noexcept
    : platform_(platform), device_(device), props_(props) {}

std::expected<DeviceManager::DeviceProps, cl_int> DeviceManager::queryProps(cl_device_id device) noexcept {
    DeviceProps props;

    const auto deviceVersionRes = getDeviceInfo<char[]>(device, CL_DEVICE_VERSION);
    if (!deviceVersionRes) return std::unexpected{deviceVersionRes.error()};
    const auto deviceVersion = std::string_view{deviceVersionRes.value().data()};
    constexpr size_t majorStartOffset = 7;
    const size_t dotOffset = deviceVersion.find_first_of('.', majorStartOffset + 1);
    const size_t secondSpaceOffset = deviceVersion.find_first_of(' ', dotOffset + 1);
    const auto deviceVersionMajorStr = deviceVersion.substr(majorStartOffset, dotOffset - majorStartOffset);
    const auto deviceVersionMinorStr = deviceVersion.substr(dotOffset + 1, secondSpaceOffset - dotOffset - 1);
    props.deviceVersionMajor = std::stoi(std::string{deviceVersionMajorStr});
    props.deviceVersionMinor = std::stoi(std::string{deviceVersionMinorStr});

    const auto deviceTypeRes = getDeviceInfo<cl_device_type>(device, CL_DEVICE_TYPE);
    if (!deviceTypeRes) return std::unexpected{deviceTypeRes.error()};
    props.deviceType = deviceTypeRes.value();

    const auto maxWorkGroupSizeRes = getDeviceInfo<size_t>(device, CL_DEVICE_MAX_WORK_GROUP_SIZE);
    if (!maxWorkGroupSizeRes) return std::unexpected{maxWorkGroupSizeRes.error()};
    props.maxWorkGroupSize = maxWorkGroupSizeRes.value();

    const auto maxWorkItemSizeRes =
        getDeviceInfo<decltype(props.maxWorkItemSize)>(device, CL_DEVICE_MAX_WORK_ITEM_SIZES);
    if (!maxWorkItemSizeRes) return std::unexpected{maxWorkItemSizeRes.error()};
    props.maxWorkItemSize = maxWorkItemSizeRes.value();

    if (props.deviceVersionMajor >= 3) {
        const auto prefferedBasicWorkGroupSizeRes =
            getDeviceInfo<size_t>(device, CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE);
        if (!prefferedBasicWorkGroupSizeRes) return std::unexpected{prefferedBasicWorkGroupSizeRes.error()};
        props.prefferedBasicWorkGroupSize = prefferedBasicWorkGroupSizeRes.value();
    }

    const auto globalMemCacheSizeRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
    if (!globalMemCacheSizeRes) return std::unexpected{globalMemCacheSizeRes.error()};
    props.globalMemCacheSize = globalMemCacheSizeRes.value();

    const auto globalMemSizeRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_GLOBAL_MEM_SIZE);
    if (!globalMemSizeRes) return std::unexpected{globalMemSizeRes.error()};
    props.globalMemSize = globalMemSizeRes.value();

    const auto maxConstBufferSizeRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
    if (!maxConstBufferSizeRes) return std::unexpected{maxConstBufferSizeRes.error()};
    props.maxConstBufferSize = maxConstBufferSizeRes.value();

    const auto localMemSizeRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_LOCAL_MEM_SIZE);
    if (!localMemSizeRes) return std::unexpected{localMemSizeRes.error()};
    props.localMemSize = localMemSizeRes.value();

    const auto maxComputeUnitsRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_MAX_COMPUTE_UNITS);
    if (!maxComputeUnitsRes) return std::unexpected{maxComputeUnitsRes.error()};
    props.maxComputeUnits = maxComputeUnitsRes.value();

    const auto globalMemCachelineSizeRes = getDeviceInfo<cl_uint>(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
    if (!globalMemCachelineSizeRes) return std::unexpected{globalMemCachelineSizeRes.error()};
    props.globalMemCachelineSize = globalMemCachelineSizeRes.value();

    const auto imagePitchAlignRes = getDeviceInfo<cl_uint>(device, CL_DEVICE_IMAGE_PITCH_ALIGNMENT);
    if (!imagePitchAlignRes) return std::unexpected{imagePitchAlignRes.error()};
    props.imagePitchAlign = imagePitchAlignRes.value();

    const auto imageBaseAddrAlignRes = getDeviceInfo<cl_uint>(device, CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT);
    if (!imageBaseAddrAlignRes) return std::unexpected{imageBaseAddrAlignRes.error()};
    props.imageBaseAddrAlign = imageBaseAddrAlignRes.value();
    props.supportImageFromBuffer = props.imagePitchAlign != 0 && props.imageBaseAddrAlign != 0;

    const auto localMemTypeRes = getDeviceInfo<cl_device_local_mem_type>(device, CL_DEVICE_LOCAL_MEM_TYPE);
    if (!localMemTypeRes) return std::unexpected{localMemTypeRes.error()};
    props.realLocalMem = localMemTypeRes.value() & CL_LOCAL;

    if (props.deviceVersionMajor >= 2 && props.deviceVersionMinor >= 1) {
        const auto supportSubGroupRes =
            getDeviceInfo<cl_bool>(device, CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS);
        if (!supportSubGroupRes) return std::unexpected{supportSubGroupRes.error()};
        props.supportSubGroup = (bool)supportSubGroupRes.value();
    }

    const auto hostQueuePropsRes =
        getDeviceInfo<cl_command_queue_properties>(device, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES);
    if (!hostQueuePropsRes) return std::unexpected{hostQueuePropsRes.error()};
    const auto& hostQueueProps = hostQueuePropsRes.value();
    props.supportOutOfOrderQueue = bool(hostQueueProps & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

    return props;
}

std::expected<DeviceManager, cl_int> DeviceManager::create() noexcept {
    const auto platformsRes = getPlatformIDs();
    if (!platformsRes) return std::unexpected{platformsRes.error()};
    const auto& platforms = platformsRes.value();

    const auto isDeviceOK = [](const cl_device_id device, const DeviceProps& props) -> std::expected<cl_bool, cl_int> {
        // const auto readWriteImageCountRes = getDeviceInfo<cl_uint>(device, CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS);
        // if (!readWriteImageCountRes) return std::unexpected{readWriteImageCountRes.error()};
        // if (readWriteImageCountRes.value() == 0) return false;

        return true;
    };

    const auto getDeviceScore = [](const cl_device_id device,
                                   const DeviceProps& props) -> std::expected<int64_t, cl_int> {
        int64_t score = 0;

        const auto deviceExtRes = getDeviceInfo<char[]>(device, CL_DEVICE_EXTENSIONS);
        if (!deviceExtRes) return std::unexpected{deviceExtRes.error()};
        const auto deviceExt = std::string_view{deviceExtRes.value().data()};
        score = rgs::count(deviceExt, ' ');

        if (props.realLocalMem) score++;
        if (props.supportSubGroup) score++;
        if (props.supportOutOfOrderQueue) score++;

        if (props.deviceType & CL_DEVICE_TYPE_GPU) score <<= 1;

        if constexpr (ENABLE_DEBUG) {
            const auto deviceNameRes = getDeviceInfo<char[]>(device, CL_DEVICE_NAME);
            if (!deviceNameRes) return std::unexpected{deviceNameRes.error()};
            const auto deviceName = std::string_view{deviceNameRes.value().data()};

            const auto deviceVersionRes = getDeviceInfo<char[]>(device, CL_DEVICE_VERSION);
            if (!deviceVersionRes) return std::unexpected{deviceVersionRes.error()};
            const auto deviceVersion = std::string_view{deviceVersionRes.value().data()};

            const auto driverVersionRes = getDeviceInfo<char[]>(device, CL_DRIVER_VERSION);
            if (!driverVersionRes) return std::unexpected{driverVersionRes.error()};
            const auto driverVersion = std::string_view{driverVersionRes.value().data()};

            std::println("Candidate device: name={}, deviceVer={} ({}.{}), driverVer={}, score={}", deviceName,
                         deviceVersion, props.deviceVersionMajor, props.deviceVersionMinor, driverVersion, score);
        }

        return score;
    };

    std::vector<Score<std::tuple<cl_platform_id, cl_device_id, DeviceProps>>> scores;
    for (const auto& platform : platforms) {
        const auto devicesRes = getDeviceIDs(platform);
        if (!devicesRes) return std::unexpected{devicesRes.error()};

        const auto& devices = devicesRes.value();
        for (const auto& device : devices) {
            auto devicePropsRes = queryProps(device);
            if (!devicePropsRes) return std::unexpected{devicePropsRes.error()};
            const auto& deviceProps = devicePropsRes.value();

            auto okRes = isDeviceOK(device, deviceProps);
            if (!okRes) return std::unexpected{okRes.error()};
            if (!okRes.value()) continue;

            auto scoreRes = getDeviceScore(device, deviceProps);
            if (!scoreRes) return std::unexpected{scoreRes.error()};
            scores.emplace_back(scoreRes.value(), std::tuple{platform, device, deviceProps});
        }
    }

    if (scores.empty()) {
        return std::unexpected{-1};
    }

    const auto maxScoreIt = std::max_element(scores.begin(), scores.end());
    auto [platform, device, props] = maxScoreIt->attachment;
    return DeviceManager{std::move(platform), std::move(device), std::move(props)};
}

}  // namespace clc
