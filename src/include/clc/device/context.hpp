#pragma once

#include <expected>

#include <CL/cl.h>

#include "clc/device/box.hpp"

namespace clc {

class ContextBox {
    ContextBox(cl_context context) noexcept;

public:
    ContextBox(ContextBox&& rhs) noexcept;
    ~ContextBox() noexcept;

    [[nodiscard]] static std::expected<ContextBox, Error> create(DeviceBox& deviceBox) noexcept;

    [[nodiscard]] cl_context getContext() const noexcept { return context_; }

private:
    cl_context context_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/context.cpp"
#endif
