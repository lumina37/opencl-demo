#include <array>
#include <cstddef>
#include <expected>
#include <functional>
#include <ranges>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/device/manager.hpp"
#include "clc/helper/error.hpp"
#include "clc/resource.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/queue.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

QueueManager::QueueManager(cl_command_queue&& queue) noexcept : queue_(queue) {}

QueueManager::QueueManager(QueueManager&& rhs) noexcept { queue_ = std::exchange(rhs.queue_, nullptr); }

QueueManager::~QueueManager() noexcept {
    if (queue_ == nullptr) return;
    clReleaseCommandQueue(queue_);
    queue_ = nullptr;
}

std::expected<QueueManager, Error> QueueManager::createWithProps(DeviceManager& deviceMgr, ContextManager& contextMgr,
                                                                 cl_queue_properties queueProps) noexcept {
    cl_int clErr;

    auto device = deviceMgr.getDevice();
    auto context = contextMgr.getContext();
    const std::array queuePropArray{(cl_queue_properties)CL_QUEUE_PROPERTIES,
                                    queueProps | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, (cl_queue_properties)0};
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, queuePropArray.data(), &clErr);

    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return QueueManager{std::move(queue)};
}

std::expected<EventManager, Error> QueueManager::uploadBufferFrom(
    BufferManager& dstBufferMgr, const std::span<std::byte> src,
    std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept {
    auto eventMgrRes = EventManager::create();
    if (!eventMgrRes) return std::unexpected{std::move(eventMgrRes.error())};
    EventManager& eventMgr = eventMgrRes.value();

    auto waitEvents = waitEventMgrs | rgs::views::transform(EventManager::leak) | rgs::to<std::vector>();

    const cl_int clErr = clEnqueueWriteBuffer(queue_, dstBufferMgr.getBuffer(), false, 0, src.size(), src.data(),
                                              waitEvents.size(), waitEvents.data(), eventMgr.getPEvent());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return std::move(eventMgr);
}

std::expected<EventManager, Error> QueueManager::uploadImageFrom(
    ImageManager& dstImageMgr, const std::span<std::byte> src, const Extent extent,
    std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept {
    auto eventMgrRes = EventManager::create();
    if (!eventMgrRes) return std::unexpected{std::move(eventMgrRes.error())};
    EventManager& eventMgr = eventMgrRes.value();

    auto waitEvents = waitEventMgrs | rgs::views::transform(EventManager::leak) | rgs::to<std::vector>();

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    const cl_int clErr =
        clEnqueueWriteImage(queue_, dstImageMgr.getImage(), false, origin.data(), region.data(), extent.rowPitch(), 0,
                            src.data(), waitEvents.size(), waitEvents.data(), eventMgr.getPEvent());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return std::move(eventMgr);
}

std::expected<EventManager, Error> QueueManager::dispatch(
    const KernelManager& kernelMgr, Extent extent, GroupSize localGroupSize,
    std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept {
    auto eventMgrRes = EventManager::create();
    if (!eventMgrRes) return std::unexpected{std::move(eventMgrRes.error())};
    EventManager& eventMgr = eventMgrRes.value();

    auto waitEvents = waitEventMgrs | rgs::views::transform(EventManager::leak) | rgs::to<std::vector>();

    const GroupSize globalGroupSize{(extent.width() + localGroupSize.x - 1) / localGroupSize.x * localGroupSize.x,
                                    (extent.height() + localGroupSize.y - 1) / localGroupSize.y * localGroupSize.y};
    const cl_int clErr =
        clEnqueueNDRangeKernel(queue_, kernelMgr.getKernel(), 2, nullptr, (size_t*)&globalGroupSize,
                               (size_t*)&localGroupSize, waitEvents.size(), waitEvents.data(), eventMgr.getPEvent());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return std::move(eventMgr);
}

std::expected<EventManager, Error> QueueManager::downloadImageTo(
    const ImageManager& srcImageMgr, std::span<std::byte> dst, const Extent extent,
    std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept {
    auto eventMgrRes = EventManager::create();
    if (!eventMgrRes) return std::unexpected{std::move(eventMgrRes.error())};
    EventManager& eventMgr = eventMgrRes.value();

    auto waitEvents = waitEventMgrs | rgs::views::transform(EventManager::leak) | rgs::to<std::vector>();

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    const cl_int clErr =
        clEnqueueReadImage(queue_, srcImageMgr.getImage(), false, origin.data(), region.data(), extent.rowPitch(), 0,
                           dst.data(), waitEvents.size(), waitEvents.data(), eventMgr.getPEvent());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return std::move(eventMgr);
}

std::expected<std::span<std::byte>, Error> QueueManager::mmapForHostRead(
    ImageManager& imageMgr, const Extent extent,
    std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept {
    cl_int clErr;

    auto waitEvents = waitEventMgrs | rgs::views::transform(EventManager::leak) | rgs::to<std::vector>();

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    size_t rowPitch;
    const void* mapPtr = clEnqueueMapImage(queue_, imageMgr.getImage(), true, CL_MAP_READ, origin.data(), region.data(),
                                           &rowPitch, nullptr, waitEvents.size(), waitEvents.data(), nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    const std::span mapSpan{(std::byte*)mapPtr, extent.size()};
    return mapSpan;
}

std::expected<std::span<std::byte>, Error> QueueManager::mmapForHostWrite(
    ImageManager& imageMgr, const Extent extent,
    std::span<std::reference_wrapper<const EventManager>> waitEventMgrs) noexcept {
    cl_int clErr;

    auto waitEvents = waitEventMgrs | rgs::views::transform(EventManager::leak) | rgs::to<std::vector>();

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    size_t rowPitch;
    const void* mapPtr =
        clEnqueueMapImage(queue_, imageMgr.getImage(), true, CL_MAP_WRITE, origin.data(), region.data(), &rowPitch,
                          nullptr, waitEvents.size(), waitEvents.data(), nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    const std::span mapSpan{(std::byte*)mapPtr, extent.size()};
    return mapSpan;
}

std::expected<void, Error> QueueManager::unmap(ImageManager& imageMgr, const std::span<std::byte> mapSpan) noexcept {
    const cl_int clErr = clEnqueueUnmapMemObject(queue_, imageMgr.getImage(), mapSpan.data(), 0, nullptr, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return {};
}

}  // namespace clc
