#pragma once

#include <cstdint>
#include <expected>
#include <type_traits>
#include <vector>

#include <CL/cl.h>

namespace clc {

static inline constexpr uint32_t packVersion(uint16_t major, uint16_t minor) noexcept {
    return (uint32_t)major << 16 | (uint32_t)minor;
}

std::expected<std::vector<cl_platform_id>, cl_int> getPlatformIDs() noexcept;

std::expected<std::vector<cl_device_id>, cl_int> getDeviceIDs(cl_platform_id platform) noexcept;

template <typename Tv>
    requires !std::is_array_v<Tv> && std::is_trivially_copyable_v<Tv>
std::expected<Tv, cl_int> getPlatformInfo(const cl_platform_id& platform, const cl_device_info& key) {
    Tv value;
    const cl_int clErr = clGetPlatformInfo(platform, key, sizeof(Tv), &value, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    return value;
}

template <typename TArr, typename TElem = std::remove_extent_t<TArr>>
    requires std::is_unbounded_array_v<TArr>
std::expected<std::vector<TElem>, cl_int> getPlatformInfo(const cl_platform_id& platform, const cl_device_info& key) {
    cl_int clErr;

    size_t valueSize;
    clErr = clGetPlatformInfo(platform, key, 0, nullptr, &valueSize);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    std::vector<TElem> value(valueSize);  // TODO: how to avoid pre-init?
    clErr = clGetPlatformInfo(platform, key, valueSize, value.data(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    return value;
}

template <typename Tv>
    requires !std::is_unbounded_array_v<Tv> && std::is_trivially_copyable_v<Tv>
std::expected<Tv, cl_int> getDeviceInfo(const cl_device_id& device, const cl_device_info& key) {
    Tv value;
    const cl_int clErr = clGetDeviceInfo(device, key, sizeof(Tv), &value, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    return value;
}

template <typename TArr, typename TElem = std::remove_extent_t<TArr>>
    requires std::is_unbounded_array_v<TArr>
std::expected<std::vector<TElem>, cl_int> getDeviceInfo(const cl_device_id& device, const cl_device_info& key) {
    cl_int clErr;

    size_t valueSize;
    clErr = clGetDeviceInfo(device, key, 0, nullptr, &valueSize);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    std::vector<TElem> value(valueSize);  // TODO: how to avoid pre-init?
    clErr = clGetDeviceInfo(device, key, valueSize, value.data(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    return value;
}

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/props.cpp"
#endif
