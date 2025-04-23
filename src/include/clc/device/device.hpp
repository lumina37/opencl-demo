#pragma once

#include <array>
#include <cstddef>

#include <CL/cl.h>

namespace clc {

class DeviceManager {
public:
    struct DeviceProps {
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
    };

private:
    DeviceManager(cl_platform_id&& platform, cl_device_id&& device, DeviceProps&& props) noexcept;

    [[nodiscard]] static std::expected<DeviceProps, cl_int> queryProps(cl_device_id device) noexcept;

public:
    [[nodiscard]] static std::expected<DeviceManager, cl_int> create() noexcept;

    [[nodiscard]] cl_device_id getDevice() const noexcept { return device_; }
    [[nodiscard]] const DeviceProps& getDeviceProps() const noexcept { return props_; }

private:
    cl_platform_id platform_;
    cl_device_id device_;
    DeviceProps props_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/device.cpp"
#endif
