#pragma once

#include <cstddef>
#include <span>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/device/device.hpp"

namespace clc {

struct KernelArg {
    size_t size;
    void* ptr;
};

class KernelManager {
public:
    KernelManager(DeviceManager& deviceMgr, ContextManager& contextMgr, std::span<std::byte> code);
    ~KernelManager();

    template <typename Self>
    [[nodiscard]] auto&& getKernel(this Self&& self) noexcept {
        return std::forward_like<Self>(self).kernel_;
    }

    void setKernelArgs(std::span<KernelArg> args);

private:
    cl_program program_;
    cl_kernel kernel_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/kernel.cpp"
#endif
