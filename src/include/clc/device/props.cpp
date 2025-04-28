#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <ranges>
#include <string>
#include <utility>

#include <CL/cl.h>

#include "clc/device/helper.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/props.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

std::expected<DeviceProps, Error> DeviceProps::create(cl_device_id device) noexcept {
    DeviceProps props;

    auto deviceVersionRes = getDeviceInfo<char[]>(device, CL_DEVICE_VERSION);
    if (!deviceVersionRes) return std::unexpected{std::move(deviceVersionRes.error())};
    const auto deviceVersion = std::string_view{deviceVersionRes.value().data()};
    constexpr size_t majorStartOffset = 7;
    const size_t dotOffset = deviceVersion.find_first_of('.', majorStartOffset + 1);
    const size_t secondSpaceOffset = deviceVersion.find_first_of(' ', dotOffset + 1);
    const auto deviceVersionMajorStr = deviceVersion.substr(majorStartOffset, dotOffset - majorStartOffset);
    const auto deviceVersionMinorStr = deviceVersion.substr(dotOffset + 1, secondSpaceOffset - dotOffset - 1);
    props.deviceVersionMajor = (uint16_t)std::stoi(std::string{deviceVersionMajorStr});
    props.deviceVersionMinor = (uint16_t)std::stoi(std::string{deviceVersionMinorStr});
    props.deviceVersion = packVersion(props.deviceVersionMajor, props.deviceVersionMinor);

    auto deviceTypeRes = getDeviceInfo<cl_device_type>(device, CL_DEVICE_TYPE);
    if (!deviceTypeRes) return std::unexpected{std::move(deviceTypeRes.error())};
    props.deviceType = deviceTypeRes.value();

    auto maxWorkGroupSizeRes = getDeviceInfo<size_t>(device, CL_DEVICE_MAX_WORK_GROUP_SIZE);
    if (!maxWorkGroupSizeRes) return std::unexpected{std::move(maxWorkGroupSizeRes.error())};
    props.maxWorkGroupSize = maxWorkGroupSizeRes.value();

    auto maxWorkItemSizeRes = getDeviceInfo<decltype(props.maxWorkItemSize)>(device, CL_DEVICE_MAX_WORK_ITEM_SIZES);
    if (!maxWorkItemSizeRes) return std::unexpected{std::move(maxWorkItemSizeRes.error())};
    props.maxWorkItemSize = maxWorkItemSizeRes.value();

    auto prefferedBasicWorkGroupSizeRes = getDeviceInfo<size_t>(device, CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE);
    if (!prefferedBasicWorkGroupSizeRes) {
        if (prefferedBasicWorkGroupSizeRes.error().code != CL_INVALID_VALUE ||
            props.deviceVersion >= packVersion(3, 0)) {
            return std::unexpected{std::move(prefferedBasicWorkGroupSizeRes.error())};
        }
    } else {
        props.prefferedBasicWorkGroupSize = prefferedBasicWorkGroupSizeRes.value();
    }

    auto globalMemCacheSizeRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
    if (!globalMemCacheSizeRes) return std::unexpected{std::move(globalMemCacheSizeRes.error())};
    props.globalMemCacheSize = globalMemCacheSizeRes.value();

    auto globalMemSizeRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_GLOBAL_MEM_SIZE);
    if (!globalMemSizeRes) return std::unexpected{std::move(globalMemSizeRes.error())};
    props.globalMemSize = globalMemSizeRes.value();

    auto maxConstBufferSizeRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
    if (!maxConstBufferSizeRes) return std::unexpected{std::move(maxConstBufferSizeRes.error())};
    props.maxConstBufferSize = maxConstBufferSizeRes.value();

    auto localMemSizeRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_LOCAL_MEM_SIZE);
    if (!localMemSizeRes) return std::unexpected{std::move(localMemSizeRes.error())};
    props.localMemSize = localMemSizeRes.value();

    auto maxComputeUnitsRes = getDeviceInfo<cl_ulong>(device, CL_DEVICE_MAX_COMPUTE_UNITS);
    if (!maxComputeUnitsRes) return std::unexpected{std::move(maxComputeUnitsRes.error())};
    props.maxComputeUnits = maxComputeUnitsRes.value();

    auto globalMemCachelineSizeRes = getDeviceInfo<cl_uint>(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
    if (!globalMemCachelineSizeRes) return std::unexpected{std::move(globalMemCachelineSizeRes.error())};
    props.globalMemCachelineSize = globalMemCachelineSizeRes.value();

    auto imagePitchAlignRes = getDeviceInfo<cl_uint>(device, CL_DEVICE_IMAGE_PITCH_ALIGNMENT);
    if (!imagePitchAlignRes) {
        if (imagePitchAlignRes.error().code != CL_INVALID_VALUE || props.deviceVersion >= packVersion(2, 0)) {
            return std::unexpected{std::move(imagePitchAlignRes.error())};
        }
    } else {
        props.imagePitchAlign = imagePitchAlignRes.value();
    }

    auto imageBaseAddrAlignRes = getDeviceInfo<cl_uint>(device, CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT);
    if (!imageBaseAddrAlignRes) {
        if (imageBaseAddrAlignRes.error().code != CL_INVALID_VALUE || props.deviceVersion >= packVersion(2, 0)) {
            return std::unexpected{std::move(imageBaseAddrAlignRes.error())};
        }
    } else {
        props.imageBaseAddrAlign = imageBaseAddrAlignRes.value();
    }
    props.supportImageFromBuffer = props.imagePitchAlign != 0 && props.imageBaseAddrAlign != 0;

    auto localMemTypeRes = getDeviceInfo<cl_device_local_mem_type>(device, CL_DEVICE_LOCAL_MEM_TYPE);
    if (!localMemTypeRes) return std::unexpected{std::move(localMemTypeRes.error())};
    props.realLocalMem = localMemTypeRes.value() & CL_LOCAL;

    auto maxSubGroupsRes = getDeviceInfo<cl_uint>(device, CL_DEVICE_MAX_NUM_SUB_GROUPS);
    if (!maxSubGroupsRes) {
        if (maxSubGroupsRes.error().code != CL_INVALID_VALUE || props.deviceVersion >= packVersion(2, 1)) {
            return std::unexpected{std::move(maxSubGroupsRes.error())};
        }
    } else {
        props.supportSubGroup = (bool)maxSubGroupsRes.value();
    }

    if (props.deviceVersion >= packVersion(2, 0)) {
        auto hostQueuePropsRes = getDeviceInfo<cl_command_queue_properties>(device, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES);
        if (!hostQueuePropsRes) return std::unexpected{std::move(hostQueuePropsRes.error())};
        const auto& hostQueueProps = hostQueuePropsRes.value();
        props.supportOutOfOrderQueue = bool(hostQueueProps & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    } else {
        auto queuePropsRes = getDeviceInfo<cl_command_queue_properties>(device, CL_DEVICE_QUEUE_PROPERTIES);
        if (!queuePropsRes) return std::unexpected{std::move(queuePropsRes.error())};
        const auto& queueProps = queuePropsRes.value();
        props.supportOutOfOrderQueue = bool(queueProps & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    }

    auto readWriteImageCountRes = getDeviceInfo<cl_uint>(device, CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS);
    if (!readWriteImageCountRes) {
        if (readWriteImageCountRes.error().code != CL_INVALID_VALUE || props.deviceVersion >= packVersion(2, 1)) {
            return std::unexpected{std::move(readWriteImageCountRes.error())};
        }
    } else {
        props.supportReadWriteImage = (bool)readWriteImageCountRes.value();
    }

    auto svmCapsRes = getDeviceInfo<cl_device_svm_capabilities>(device, CL_DEVICE_SVM_CAPABILITIES);
    if (!svmCapsRes) {
        if (svmCapsRes.error().code != CL_INVALID_VALUE || props.deviceVersion >= packVersion(2, 0)) {
            return std::unexpected{std::move(svmCapsRes.error())};
        }
    } else {
        const auto svmCaps = svmCapsRes.value();
        props.supportCoarseSVM = (bool)(svmCaps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER);
    }

    auto deviceExtRes = getDeviceInfo<char[]>(device, CL_DEVICE_EXTENSIONS);
    if (!deviceExtRes) return std::unexpected{std::move(deviceExtRes.error())};
    props.extensionStr = std::string{deviceExtRes.value().data()};
    props.extensions =
        props.extensionStr | rgs::views::split(' ') | rgs::views::filter(std::not_fn(rgs::empty)) |
        rgs::views::transform([](const auto& subRange) { return std::string_view{subRange.begin(), subRange.end()}; }) |
        rgs::to<std::vector>();
    rgs::sort(props.extensions);

    return props;
}

bool DeviceProps::hasExtension(std::string_view extName) const noexcept {
    return rgs::binary_search(extensions, extName);
}

template class DeviceWithProps_<>;
using DeviceWithProps = DeviceWithProps_<>;

}  // namespace clc
