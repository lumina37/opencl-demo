#pragma once

#include <cstddef>
#include <expected>
#include <functional>
#include <span>

#include <CL/cl.h>

#include "clc/device/box.hpp"
#include "clc/device/context.hpp"
#include "clc/event.hpp"
#include "clc/kernel.hpp"
#include "clc/resource.hpp"

namespace clc {

struct GroupSize {
    size_t x, y;
};

class QueueBox {
    QueueBox(cl_command_queue queue) noexcept;

public:
    QueueBox(QueueBox&&) noexcept;
    ~QueueBox() noexcept;

    [[nodiscard]] static std::expected<QueueBox, Error> createWithProps(DeviceBox& deviceBox, ContextBox& contextBox,
                                                                        cl_queue_properties queueProps) noexcept;

    [[nodiscard]] cl_command_queue getQueue() const noexcept { return queue_; }

    [[nodiscard]] std::expected<EventBox, Error> uploadBufferFrom(
        BufferBox& dstBufferBox, std::span<std::byte> src,
        std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept;

    [[nodiscard]] std::expected<EventBox, Error> uploadImageFrom(
        ImageBox& dstImageBox, std::span<std::byte> src, Extent extent,
        std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept;

    [[nodiscard]] std::expected<EventBox, Error> dispatch(
        const KernelBox& kernelBox, Extent extent, GroupSize localGroupSize,
        std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept;

    [[nodiscard]] std::expected<EventBox, Error> downloadBufferTo(
        const BufferBox& srcBufferBox, std::span<std::byte> dst,
        std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept;

    [[nodiscard]] std::expected<EventBox, Error> downloadImageTo(
        const ImageBox& srcImageBox, std::span<std::byte> dst, Extent extent,
        std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept;

    [[nodiscard]] std::expected<std::span<std::byte>, Error> mmapForHostRead(
        ImageBox& imageBox, Extent extent, std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept;

    [[nodiscard]] std::expected<std::span<std::byte>, Error> mmapForHostWrite(
        ImageBox& imageBox, Extent extent, std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept;

    [[nodiscard]] std::expected<void, Error> unmap(ImageBox& imageBox, std::span<std::byte> mapSpan) noexcept;

private:
    cl_command_queue queue_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/queue.cpp"
#endif
