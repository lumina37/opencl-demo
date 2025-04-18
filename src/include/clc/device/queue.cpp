#include <array>
#include <expected>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/device/device.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/queue.hpp"
#endif

namespace clc {

QueueManager::QueueManager(cl_command_queue&& queue) noexcept : queue_(queue) {}

QueueManager::QueueManager(QueueManager&& rhs) noexcept { queue_ = std::exchange(rhs.queue_, nullptr); }

QueueManager::~QueueManager() noexcept {
    if (queue_ == nullptr) return;
    clReleaseCommandQueue(queue_);
    queue_ = nullptr;
}

std::expected<QueueManager, cl_int> QueueManager::createWithProps(DeviceManager& deviceMgr, ContextManager& contextMgr,
                                                                  cl_queue_properties queueProps) noexcept {
    cl_int clErr;

    auto device = deviceMgr.getDevice();
    auto context = contextMgr.getContext();
    const std::array realQueueProps{(cl_queue_properties)CL_QUEUE_PROPERTIES,
                                    queueProps | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, (cl_queue_properties)0};
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, realQueueProps.data(), &clErr);

    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    return QueueManager{std::move(queue)};
}

}  // namespace clc
