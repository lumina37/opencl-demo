#pragma once

#include <CL/cl.h>

#include "clc/device/device.hpp"

namespace clc {

class ContextManager {
public:
    ContextManager(DeviceManager& deviceMgr);
    ~ContextManager();

    [[nodiscard]] cl_context getContext() const noexcept { return context_; }

private:
    cl_context context_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/context.cpp"
#endif
