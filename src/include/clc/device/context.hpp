#pragma once

#include <expected>

#include <CL/cl.h>

#include "clc/device/manager.hpp"

namespace clc {

class ContextManager {
    ContextManager(cl_context&& context) noexcept;

public:
    ContextManager(ContextManager&&) noexcept;
    ~ContextManager() noexcept;

    [[nodiscard]] static std::expected<ContextManager, Error> create(DeviceManager& deviceMgr) noexcept;

    [[nodiscard]] cl_context getContext() const noexcept { return context_; }

private:
    cl_context context_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/context.cpp"
#endif
