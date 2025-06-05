#pragma once

#include <cstddef>
#include <expected>
#include <span>
#include <utility>

#include <CL/cl.h>

#include "clc/device/box.hpp"
#include "clc/device/context.hpp"
#include "clc/kernel/structs.hpp"

namespace clc {

class KernelBox {
    KernelBox(cl_program program, cl_kernel kernel) noexcept;

public:
    KernelBox(KernelBox&& rhs) noexcept;
    ~KernelBox() noexcept;

    [[nodiscard]] static std::expected<KernelBox, Error> create(DeviceBox& deviceBox, ContextBox& contextBox,
                                                                std::span<std::byte> code) noexcept;

    template <typename Self>
    [[nodiscard]] auto&& getKernel(this Self&& self) noexcept {
        return std::forward_like<Self>(self).kernel_;
    }

    [[nodiscard]] std::expected<void, Error> setKernelArgs(std::span<const KernelArg> args) noexcept;

private:
    cl_program program_;
    cl_kernel kernel_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/kernel/box.cpp"
#endif
