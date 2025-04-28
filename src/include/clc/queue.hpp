#pragma once

#include <cstddef>
#include <expected>
#include <functional>
#include <span>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/device/manager.hpp"
#include "clc/event.hpp"
#include "clc/kernel.hpp"
#include "clc/resource.hpp"

namespace clc {

struct GroupSize {
    size_t x, y;
};

class QueueManager {
    QueueManager(cl_command_queue&& queue) noexcept;

public:
    QueueManager(QueueManager&&) noexcept;
    ~QueueManager() noexcept;

    [[nodiscard]] static std::expected<QueueManager, Error> createWithProps(DeviceManager& deviceMgr,
                                                                            ContextManager& contextMgr,
                                                                            cl_queue_properties queueProps) noexcept;

    [[nodiscard]] cl_command_queue getQueue() const noexcept { return queue_; }

    [[nodiscard]] std::expected<EventManager, Error> uploadBufferFrom(
        BufferManager& dstBufferMgr, std::span<std::byte> src,
        std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept;

    [[nodiscard]] std::expected<EventManager, Error> uploadImageFrom(
        ImageManager& dstImageMgr, std::span<std::byte> src, Extent extent,
        std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept;

    [[nodiscard]] std::expected<EventManager, Error> dispatch(
        const KernelManager& kernelMgr, Extent extent, GroupSize localGroupSize,
        std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept;

    [[nodiscard]] std::expected<EventManager, Error> downloadImageTo(
        const ImageManager& srcImageMgr, std::span<std::byte> dst, Extent extent,
        std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept;

    [[nodiscard]] std::expected<std::span<std::byte>, Error> mmapForHostRead(
        ImageManager& imageMgr, Extent extent,
        std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept;

    [[nodiscard]] std::expected<std::span<std::byte>, Error> mmapForHostWrite(
        ImageManager& imageMgr, Extent extent,
        std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept;

    [[nodiscard]] std::expected<void, Error> unmap(ImageManager& imageMgr, std::span<std::byte> mapSpan) noexcept;

private:
    cl_command_queue queue_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/device/queue.cpp"
#endif
