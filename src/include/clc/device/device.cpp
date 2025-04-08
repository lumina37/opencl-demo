#include <memory>

#include <CL/cl.h>

#include "clc/device/platform.hpp"
#include "clc/helper/exception.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/device.hpp"
#endif

namespace clc {

DeviceManager::DeviceManager(PlatformManager& platformMgr) {
    cl_uint err;

    cl_uint deviceCount;
    err = clGetDeviceIDs(platformMgr.getPlatform(), CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
    checkError(err);

    auto pDevices = std::make_unique_for_overwrite<cl_device_id[]>(deviceCount);
    err = clGetDeviceIDs(platformMgr.getPlatform(), CL_DEVICE_TYPE_ALL, deviceCount, pDevices.get(), nullptr);
    checkError(err);

    device_ = pDevices[0];
}

}  // namespace clc
