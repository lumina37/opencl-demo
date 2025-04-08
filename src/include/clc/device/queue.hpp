#pragma once

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/device/device.hpp"

namespace clc {

class QueueManager {
public:
    QueueManager(DeviceManager& deviceMgr, ContextManager& contextMgr);
    ~QueueManager();

    [[nodiscard]] cl_command_queue getQueue() const noexcept { return queue_; }

private:
    cl_command_queue queue_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/queue.cpp"
#endif
