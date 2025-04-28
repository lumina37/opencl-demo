#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <utility>
#include <vector>

#include <CL/cl.h>

#include "clc/device/concepts.hpp"
#include "clc/device/manager.hpp"
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
    bool supportImage;
    bool supportReadWriteImage;
    bool supportImageFromBuffer;
    bool supportCoarseSVM;
    std::string extensionStr;
    std::vector<std::string_view> extensions;
};

template <CDeviceProps TProps_>
class DeviceWithProps_ {
public:
    using TProps = TProps_;

    DeviceWithProps_(DeviceManager&& manager, TProps&& props) noexcept;

    template <typename Self>
    [[nodiscard]] auto&& getManager(this Self&& self) noexcept {
        return std::forward_like<Self>(self).manager_;
    }
    [[nodiscard]] const TProps& getProps() const noexcept { return props_; }

private:
    DeviceManager manager_;
    TProps props_;
};

template <CDeviceProps TProps>
DeviceWithProps_<TProps>::DeviceWithProps_(DeviceManager&& manager, TProps&& props) noexcept
    : manager_(std::move(manager)), props_(std::move(props)) {}

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/props.cpp"
#endif
