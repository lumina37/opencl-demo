#include <array>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/device/device.hpp"
#include "clc/helper/exception.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/queue.hpp"
#endif

namespace clc {

QueueManager::QueueManager(DeviceManager& deviceMgr, ContextManager& contextMgr, const cl_queue_properties queueProps) {
    cl_int errCode;

    auto device = deviceMgr.getDevice();
    auto context = contextMgr.getContext();
    const std::array realQueueProps{(cl_queue_properties)CL_QUEUE_PROPERTIES,
                                    queueProps | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, (cl_queue_properties)0};
    queue_ = clCreateCommandQueueWithProperties(context, device, realQueueProps.data(), &errCode);
    checkError(errCode);
}

QueueManager::~QueueManager() { clReleaseCommandQueue(queue_); }

}  // namespace clc
