#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <type_traits>
#include <vector>

#include <CL/cl.h>

#include "clc/helper/error.hpp"

namespace clc {

static constexpr uint32_t packVersion(uint16_t major, uint16_t minor) noexcept {
    return (uint32_t)major << 16 | (uint32_t)minor;
}

static std::pair<uint16_t, uint16_t> parseVersion(std::string_view versionStr) noexcept {
    constexpr size_t majorStartOffset = 7;
    const size_t dotOffset = versionStr.find_first_of('.', majorStartOffset + 1);
    const size_t secondSpaceOffset = versionStr.find_first_of(' ', dotOffset + 1);
    const auto deviceVersionMajorStr = versionStr.substr(majorStartOffset, dotOffset - majorStartOffset);
    const auto deviceVersionMinorStr = versionStr.substr(dotOffset + 1, secondSpaceOffset - dotOffset - 1);
    const uint16_t major = (uint16_t)std::stoi(std::string{deviceVersionMajorStr});
    const uint16_t minor = (uint16_t)std::stoi(std::string{deviceVersionMinorStr});
    return {major, minor};
}

std::expected<std::vector<cl_platform_id>, Error> getPlatformIDs() noexcept;

std::expected<std::vector<cl_device_id>, Error> getDeviceIDs(cl_platform_id platform) noexcept;

template <typename Tv>
    requires(!std::is_unbounded_array_v<Tv>)
std::expected<Tv, Error> getPlatformInfo(const cl_platform_id& platform, const cl_device_info& key) {
    Tv value;
    const cl_int clErr = clGetPlatformInfo(platform, key, sizeof(Tv), &value, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};
    return value;
}

template <typename TArr, typename TElem = std::remove_extent_t<TArr>>
    requires std::is_unbounded_array_v<TArr>
std::expected<std::vector<TElem>, Error> getPlatformInfo(const cl_platform_id& platform, const cl_device_info& key) {
    cl_int clErr;

    size_t valueSize;
    clErr = clGetPlatformInfo(platform, key, 0, nullptr, &valueSize);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};

    std::vector<TElem> value(valueSize);  // TODO: how to avoid pre-init?
    clErr = clGetPlatformInfo(platform, key, valueSize, value.data(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};

    return value;
}

template <typename Tv>
    requires(!std::is_unbounded_array_v<Tv>)
std::expected<Tv, Error> getDeviceInfo(const cl_device_id& device, const cl_device_info& key) {
    Tv value;
    const cl_int clErr = clGetDeviceInfo(device, key, sizeof(Tv), &value, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};
    return value;
}

template <typename TArr, typename TElem = std::remove_extent_t<TArr>>
    requires std::is_unbounded_array_v<TArr>
std::expected<std::vector<TElem>, Error> getDeviceInfo(const cl_device_id& device, const cl_device_info& key) {
    cl_int clErr;

    size_t valueSize;
    clErr = clGetDeviceInfo(device, key, 0, nullptr, &valueSize);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};

    std::vector<TElem> value(valueSize);  // TODO: how to avoid pre-init?
    clErr = clGetDeviceInfo(device, key, valueSize, value.data(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{ECate::eCL, clErr}};

    return value;
}

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/helper.cpp"
#endif
