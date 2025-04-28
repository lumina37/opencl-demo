#pragma once

#include <cstddef>
#include <expected>
#include <span>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/device/manager.hpp"
#include "clc/kernel/structs.hpp"

namespace clc {

class KernelManager {
    KernelManager(cl_program&& program, cl_kernel&& kernel) noexcept;

public:
    KernelManager(KernelManager&& rhs) noexcept;
    ~KernelManager() noexcept;

    [[nodiscard]] static std::expected<KernelManager, Error> create(DeviceManager& deviceMgr,
                                                                     ContextManager& contextMgr,
                                                                     std::span<std::byte> code) noexcept;

    template <typename Self>
    [[nodiscard]] auto&& getKernel(this Self&& self) noexcept {
        return std::forward_like<Self>(self).kernel_;
    }

    [[nodiscard]] std::expected<void, Error> setKernelArgs(std::span<KernelArg> args) noexcept;

private:
    cl_program program_;
    cl_kernel kernel_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/kernel/manager.cpp"
#endif
