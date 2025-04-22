#include <expected>
#include <memory>
#include <print>
#include <ranges>
#include <utility>
#include <vector>

#include <CL/cl.h>

#include "clc/common/defines.hpp"
#include "clc/device/platform.hpp"
#include "clc/helper/score.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/device.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

DeviceManager::DeviceManager(cl_device_id&& device) noexcept : device_(device) {}

std::expected<DeviceManager, cl_int> DeviceManager::create(PlatformManager& platformMgr) noexcept {
    cl_uint clErr;

    cl_uint deviceCount;
    clErr = clGetDeviceIDs(platformMgr.getPlatform(), CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    auto pDevices = std::make_unique_for_overwrite<cl_device_id[]>(deviceCount);
    clErr = clGetDeviceIDs(platformMgr.getPlatform(), CL_DEVICE_TYPE_ALL, deviceCount, pDevices.get(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    const auto isDeviceOK = [](cl_device_id device) -> std::expected<cl_bool, cl_int> {
        cl_int clErr;

        cl_bool deviceAvaliable;
        clErr = clGetDeviceInfo(device, CL_DEVICE_AVAILABLE, sizeof(deviceAvaliable), &deviceAvaliable, nullptr);
        if (clErr != CL_SUCCESS) return std::unexpected{clErr};
        if (deviceAvaliable == false) return false;

        cl_bool imageSupport;
        clErr = clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(imageSupport), &imageSupport, nullptr);
        if (clErr != CL_SUCCESS) return std::unexpected{clErr};
        if (imageSupport == false) return false;

        cl_uint readWriteImageCount;
        clErr = clGetDeviceInfo(device, CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS, sizeof(readWriteImageCount),
                                &readWriteImageCount, nullptr);
        if (clErr != CL_SUCCESS) return std::unexpected{clErr};
        if (readWriteImageCount == 0) return false;

        return true;
    };

    const auto getDeviceScore = [](cl_device_id device) -> std::expected<int64_t, cl_int> {
        cl_int clErr;
        int64_t score = 0;

        cl_command_queue_properties hostQueueProps;
        clErr = clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES, sizeof(hostQueueProps), &hostQueueProps,
                                nullptr);
        if (clErr != CL_SUCCESS) return std::unexpected{clErr};
        if (hostQueueProps & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) score++;

        cl_command_queue_properties deviceQueueProps;
        clErr = clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES, sizeof(deviceQueueProps),
                                &deviceQueueProps, nullptr);
        if (clErr != CL_SUCCESS) return std::unexpected{clErr};
        if (deviceQueueProps & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) score++;

        cl_device_svm_capabilities svmProps;
        clErr = clGetDeviceInfo(device, CL_DEVICE_SVM_CAPABILITIES, sizeof(svmProps), &svmProps, nullptr);
        if (clErr != CL_SUCCESS) return std::unexpected{clErr};
        if (svmProps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER) score++;
        if (svmProps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER) score++;
        if (svmProps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM) score++;

        return score;
    };

    std::vector<Score<int>> scores;
    scores.reserve(deviceCount);
    for (const int idx : rgs::views::iota(0, (int)deviceCount)) {
        cl_device_id device = pDevices[idx];

        auto okRes = isDeviceOK(device);
        if (!okRes) return std::unexpected{okRes.error()};
        if (!okRes.value()) continue;

        auto scoreRes = getDeviceScore(device);
        if (!scoreRes) return std::unexpected{scoreRes.error()};
        scores.emplace_back(scoreRes.value(), idx);
    }

    const auto maxScoreIt = std::max_element(scores.begin(), scores.end());
    const size_t deviceIdx = maxScoreIt->attachment;

    cl_device_id device = pDevices[deviceIdx];
    return DeviceManager{std::move(device)};
}

}  // namespace clc
