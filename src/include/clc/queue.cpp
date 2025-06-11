#include <array>
#include <cstddef>
#include <expected>
#include <functional>
#include <ranges>
#include <utility>

#include <CL/cl.h>

#include "clc/device/box.hpp"
#include "clc/device/context.hpp"
#include "clc/helper/error.hpp"
#include "clc/resource.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/queue.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

QueueBox::QueueBox(cl_command_queue queue) noexcept : queue_(queue) {}

QueueBox::QueueBox(QueueBox&& rhs) noexcept : queue_(std::exchange(rhs.queue_, nullptr)) {}

QueueBox& QueueBox::operator=(QueueBox&& rhs) noexcept {
    queue_ = std::exchange(rhs.queue_, nullptr);
    return *this;
}

QueueBox::~QueueBox() noexcept {
    if (queue_ == nullptr) return;
    clReleaseCommandQueue(queue_);
    queue_ = nullptr;
}

std::expected<QueueBox, Error> QueueBox::createWithProps(DeviceBox& deviceBox, ContextBox& contextBox,
                                                         cl_queue_properties queueProps) noexcept {
    cl_int clErr;

    auto device = deviceBox.getDevice();
    auto context = contextBox.getContext();
    const std::array queuePropArray{(cl_queue_properties)CL_QUEUE_PROPERTIES,
                                    queueProps | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, (cl_queue_properties)0};
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, queuePropArray.data(), &clErr);

    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return QueueBox{queue};
}

std::expected<EventBox, Error> QueueBox::uploadBufferFrom(
    BufferBox& dstBufferBox, const std::span<std::byte> src,
    std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept {
    auto eventBoxRes = EventBox::create();
    if (!eventBoxRes) return std::unexpected{std::move(eventBoxRes.error())};
    EventBox& eventBox = eventBoxRes.value();

    auto waitEvents = waitEventBoxs | rgs::views::transform(EventBox::exposeEvent) | rgs::to<std::vector>();

    const cl_int clErr = clEnqueueWriteBuffer(queue_, dstBufferBox.getBuffer(), false, 0, src.size(), src.data(),
                                              waitEvents.size(), waitEvents.data(), eventBox.getPEvent());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return std::move(eventBox);
}

std::expected<EventBox, Error> QueueBox::uploadImageFrom(
    ImageBox& dstImageBox, const std::span<std::byte> src, const Extent extent,
    std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept {
    auto eventBoxRes = EventBox::create();
    if (!eventBoxRes) return std::unexpected{std::move(eventBoxRes.error())};
    EventBox& eventBox = eventBoxRes.value();

    auto waitEvents = waitEventBoxs | rgs::views::transform(EventBox::exposeEvent) | rgs::to<std::vector>();

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    const cl_int clErr =
        clEnqueueWriteImage(queue_, dstImageBox.getImage(), false, origin.data(), region.data(), extent.rowPitch(), 0,
                            src.data(), waitEvents.size(), waitEvents.data(), eventBox.getPEvent());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return std::move(eventBox);
}

std::expected<EventBox, Error> QueueBox::dispatch(
    const KernelBox& kernelBox, Extent extent, GroupSize localGroupSize,
    std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept {
    auto eventBoxRes = EventBox::create();
    if (!eventBoxRes) return std::unexpected{std::move(eventBoxRes.error())};
    EventBox& eventBox = eventBoxRes.value();

    auto waitEvents = waitEventBoxs | rgs::views::transform(EventBox::exposeEvent) | rgs::to<std::vector>();

    const GroupSize globalGroupSize{(extent.width() + localGroupSize.x - 1) / localGroupSize.x * localGroupSize.x,
                                    (extent.height() + localGroupSize.y - 1) / localGroupSize.y * localGroupSize.y};
    const cl_int clErr =
        clEnqueueNDRangeKernel(queue_, kernelBox.getKernel(), 2, nullptr, (size_t*)&globalGroupSize,
                               (size_t*)&localGroupSize, waitEvents.size(), waitEvents.data(), eventBox.getPEvent());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return std::move(eventBox);
}

std::expected<EventBox, Error> QueueBox::downloadBufferTo(
    const BufferBox& srcBufferBox, std::span<std::byte> dst,
    std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept {
    auto eventBoxRes = EventBox::create();
    if (!eventBoxRes) return std::unexpected{std::move(eventBoxRes.error())};
    EventBox& eventBox = eventBoxRes.value();

    auto waitEvents = waitEventBoxs | rgs::views::transform(EventBox::exposeEvent) | rgs::to<std::vector>();

    const cl_int clErr = clEnqueueReadBuffer(queue_, srcBufferBox.getBuffer(), false, 0, dst.size(), dst.data(),
                                             waitEvents.size(), waitEvents.data(), eventBox.getPEvent());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return std::move(eventBox);
}

std::expected<EventBox, Error> QueueBox::downloadImageTo(
    const ImageBox& srcImageBox, std::span<std::byte> dst, const Extent extent,
    std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept {
    auto eventBoxRes = EventBox::create();
    if (!eventBoxRes) return std::unexpected{std::move(eventBoxRes.error())};
    EventBox& eventBox = eventBoxRes.value();

    auto waitEvents = waitEventBoxs | rgs::views::transform(EventBox::exposeEvent) | rgs::to<std::vector>();

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    const cl_int clErr =
        clEnqueueReadImage(queue_, srcImageBox.getImage(), false, origin.data(), region.data(), extent.rowPitch(), 0,
                           dst.data(), waitEvents.size(), waitEvents.data(), eventBox.getPEvent());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return std::move(eventBox);
}

std::expected<std::span<std::byte>, Error> QueueBox::mmapForHostRead(
    ImageBox& imageBox, const Extent extent, std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept {
    cl_int clErr;

    auto waitEvents = waitEventBoxs | rgs::views::transform(EventBox::exposeEvent) | rgs::to<std::vector>();

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    size_t rowPitch;
    const void* mapPtr = clEnqueueMapImage(queue_, imageBox.getImage(), true, CL_MAP_READ, origin.data(), region.data(),
                                           &rowPitch, nullptr, waitEvents.size(), waitEvents.data(), nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    const std::span mapSpan{(std::byte*)mapPtr, extent.size()};
    return mapSpan;
}

std::expected<std::span<std::byte>, Error> QueueBox::mmapForHostWrite(
    ImageBox& imageBox, const Extent extent, std::span<std::reference_wrapper<const EventBox>> waitEventBoxs) noexcept {
    cl_int clErr;

    auto waitEvents = waitEventBoxs | rgs::views::transform(EventBox::exposeEvent) | rgs::to<std::vector>();

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    size_t rowPitch;
    const void* mapPtr =
        clEnqueueMapImage(queue_, imageBox.getImage(), true, CL_MAP_WRITE, origin.data(), region.data(), &rowPitch,
                          nullptr, waitEvents.size(), waitEvents.data(), nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    const std::span mapSpan{(std::byte*)mapPtr, extent.size()};
    return mapSpan;
}

std::expected<void, Error> QueueBox::unmap(ImageBox& imageBox, const std::span<std::byte> mapSpan) noexcept {
    const cl_int clErr = clEnqueueUnmapMemObject(queue_, imageBox.getImage(), mapSpan.data(), 0, nullptr, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return {};
}

}  // namespace clc
