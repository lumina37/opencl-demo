#pragma once

#include <array>
#include <cstddef>
#include <set>
#include <string>

#include <CL/cl.h>

#include "clc/helper/error.hpp"

namespace clc {

class DeviceProps {
public:
    DeviceProps() noexcept = default;
    DeviceProps(const DeviceProps&) = delete;
    DeviceProps(DeviceProps&&) noexcept = default;

    [[nodiscard]] static std::expected<DeviceProps, Error> create(cl_device_id device) noexcept;

    [[nodiscard]] bool hasExtension(std::string_view extName) const noexcept;

    // Members
    cl_device_type deviceType;
    size_t maxWorkGroupSize;
    std::array<size_t, 3> maxWorkItemSize;
    size_t prefferedBasicWorkGroupSize;
    cl_ulong globalMemCacheSize;
    cl_ulong globalMemSize;
    cl_ulong maxConstBufferSize;
    cl_ulong localMemSize;
    cl_uint maxComputeUnits;
    cl_uint globalMemCachelineSize;
    cl_uint imagePitchAlign;
    cl_uint imageBaseAddrAlign;
    uint32_t deviceVersion;
    uint16_t deviceVersionMajor;
    uint16_t deviceVersionMinor;
    bool realLocalMem;
    bool supportSubGroup;
    bool supportOutOfOrderQueue;
    bool supportReadWriteImage;
    bool supportImageFromBuffer;
    bool supportCoarseSVM;
    std::string extensionStr;
    std::set<std::string_view> extensions;
};

class DeviceManager {
    DeviceManager(cl_platform_id&& platform, cl_device_id&& device) noexcept;

public:
    [[nodiscard]] static std::expected<DeviceManager, Error> create() noexcept;

    [[nodiscard]] cl_device_id getDevice() const noexcept { return device_; }

private:
    cl_platform_id platform_;
    cl_device_id device_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/device.cpp"
#endif
