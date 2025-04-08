#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/device/device.hpp"
#include "clc/helper/exception.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/queue.hpp"
#endif

namespace clc {

QueueManager::QueueManager(DeviceManager& deviceMgr, ContextManager& contextMgr) {
    cl_int errCode;

    auto device = deviceMgr.getDevice();
    auto context = contextMgr.getContext();
    queue_ = clCreateCommandQueueWithProperties(context, device, nullptr, &errCode);
    checkError(errCode);
}

QueueManager::~QueueManager() { clReleaseCommandQueue(queue_); }

}  // namespace clc
