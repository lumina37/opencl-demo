#include <CL/cl.h>

#include "clc/device/device.hpp"
#include "clc/helper/exception.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/context.hpp"
#endif

namespace clc {

ContextManager::ContextManager(DeviceManager& deviceMgr) {
    cl_int errCode;

    auto device = deviceMgr.getDevice();
    context_ = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &errCode);
    checkError(errCode);
}

ContextManager::~ContextManager() { clReleaseContext(context_); }

}  // namespace clc
