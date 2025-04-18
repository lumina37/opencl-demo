#include <expected>
#include <memory>
#include <utility>

#include <CL/cl.h>

#include "clc/device/platform.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/device.hpp"
#endif

namespace clc {

DeviceManager::DeviceManager(cl_device_id&& device) noexcept : device_(device) {}

std::expected<DeviceManager, cl_int> DeviceManager::create(PlatformManager& platformMgr) noexcept {
    cl_uint clErr;

    cl_uint deviceCount;
    clErr = clGetDeviceIDs(platformMgr.getPlatform(), CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    auto pDevices = std::make_unique_for_overwrite<cl_device_id[]>(deviceCount);
    clErr = clGetDeviceIDs(platformMgr.getPlatform(), CL_DEVICE_TYPE_ALL, deviceCount, pDevices.get(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    cl_device_id device = pDevices[0];
    return DeviceManager{std::move(device)};
}

}  // namespace clc
