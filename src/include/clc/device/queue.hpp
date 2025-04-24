#pragma once

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/device/device.hpp"

namespace clc {

class QueueManager {
    QueueManager(cl_command_queue&& queue) noexcept;

public:
    QueueManager(QueueManager&&) noexcept;
    ~QueueManager() noexcept;

    [[nodiscard]] static std::expected<QueueManager, Error> createWithProps(DeviceManager& deviceMgr,
                                                                             ContextManager& contextMgr,
                                                                             cl_queue_properties queueProps) noexcept;

    [[nodiscard]] cl_command_queue getQueue() const noexcept { return queue_; }

private:
    cl_command_queue queue_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/queue.cpp"
#endif
