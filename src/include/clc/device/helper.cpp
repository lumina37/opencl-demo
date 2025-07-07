#include <expected>
#include <vector>

#include <CL/cl.h>

#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/helper.hpp"
#endif

namespace clc {

std::expected<std::vector<cl_platform_id>, Error> getPlatformIDs() noexcept {
    cl_int clErr;

    cl_uint platformCount;
    clErr = clGetPlatformIDs(0, nullptr, &platformCount);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};

    std::vector<cl_platform_id> platforms(platformCount);
    clErr = clGetPlatformIDs(platformCount, platforms.data(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};

    return platforms;
}

std::expected<std::vector<cl_device_id>, Error> getDeviceIDs(const cl_platform_id platform) noexcept {
    cl_int clErr;

    cl_uint deviceCount;
    clErr = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};

    std::vector<cl_device_id> devices(deviceCount);
    clErr = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, deviceCount, devices.data(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};

    return devices;
}

}  // namespace clc
