#include <array>
#include <cstddef>
#include <utility>

#include <CL/cl.h>

#include "clc/device/queue.hpp"
#include "clc/extent.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/command_buffer.hpp"
#endif

namespace clc {

CommandBufferManager::CommandBufferManager(std::shared_ptr<QueueManager>&& pQueueMgr) noexcept
    : pQueueMgr_(pQueueMgr) {}

std::expected<CommandBufferManager, Error> CommandBufferManager::create(
    const std::shared_ptr<QueueManager>& pQueueMgr) noexcept {
    auto copiedPQueueMgr = pQueueMgr;
    return CommandBufferManager{std::move(copiedPQueueMgr)};
}

std::expected<void, Error> CommandBufferManager::uploadBufferFrom(BufferManager& dstBufferMgr,
                                                                  const std::span<std::byte> src) noexcept {
    preEvs_.emplace_back();
    const cl_int clErr = clEnqueueWriteBuffer(pQueueMgr_->getQueue(), dstBufferMgr.getBuffer(), false, 0, src.size(),
                                              src.data(), 0, nullptr, &preEvs_.back());

    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return {};
}

std::expected<void, Error> CommandBufferManager::uploadImageFrom(ImageManager& dstImageMgr,
                                                                 const std::span<std::byte> src,
                                                                 const Extent extent) noexcept {
    preEvs_.emplace_back();
    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    const cl_int clErr =
        clEnqueueWriteImage(pQueueMgr_->getQueue(), dstImageMgr.getImage(), false, origin.data(), region.data(),
                            extent.rowPitch(), 0, src.data(), 0, nullptr, &preEvs_.back());

    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return {};
}

std::expected<void, Error> CommandBufferManager::dispatch(const KernelManager& kernelMgr, Extent extent,
                                                          GroupSize localGroupSize) noexcept {
    const GroupSize globalGroupSize{(extent.width() + localGroupSize.x - 1) / localGroupSize.x * localGroupSize.x,
                                    (extent.height() + localGroupSize.y - 1) / localGroupSize.y * localGroupSize.y};
    const cl_int clErr =
        clEnqueueNDRangeKernel(pQueueMgr_->getQueue(), kernelMgr.getKernel(), 2, nullptr, (size_t*)&globalGroupSize,
                               (size_t*)&localGroupSize, preEvs_.size(), preEvs_.data(), &dispatchEv_);

    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return {};
}

std::expected<void, Error> CommandBufferManager::downloadImageTo(const ImageManager& srcImageMgr,
                                                                 std::span<std::byte> dst,
                                                                 const Extent extent) noexcept {
    postEvs_.emplace_back();
    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    const cl_int clErr =
        clEnqueueReadImage(pQueueMgr_->getQueue(), srcImageMgr.getImage(), false, origin.data(), region.data(),
                           extent.rowPitch(), 0, dst.data(), 1, &dispatchEv_, &postEvs_.back());

    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return {};
}

std::expected<void, Error> CommandBufferManager::waitTransferComplete() noexcept {
    const cl_int clErr = clWaitForEvents(postEvs_.size(), postEvs_.data());
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return {};
}

std::expected<std::span<std::byte>, Error> CommandBufferManager::mmapForHostRead(ImageManager& imageMgr,
                                                                                 const Extent extent) noexcept {
    cl_int clErr;

    preEvs_.emplace_back();
    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    size_t rowPitch;
    const void* mapPtr =
        clEnqueueMapImage(pQueueMgr_->getQueue(), imageMgr.getImage(), true, CL_MAP_READ, origin.data(), region.data(),
                          &rowPitch, nullptr, 1, &dispatchEv_, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    const std::span mapSpan{(std::byte*)mapPtr, extent.size()};
    return mapSpan;
}

std::expected<std::span<std::byte>, Error> CommandBufferManager::mmapForHostWrite(ImageManager& imageMgr,
                                                                                  const Extent extent) noexcept {
    cl_int clErr;

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    size_t rowPitch;
    const void* mapPtr =
        clEnqueueMapImage(pQueueMgr_->getQueue(), imageMgr.getImage(), true, CL_MAP_WRITE, origin.data(), region.data(),
                          &rowPitch, nullptr, 0, nullptr, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    const std::span mapSpan{(std::byte*)mapPtr, extent.size()};
    return mapSpan;
}

std::expected<void, Error> CommandBufferManager::unmap(ImageManager& imageMgr,
                                                       const std::span<std::byte> mapSpan) noexcept {
    const cl_int clErr =
        clEnqueueUnmapMemObject(pQueueMgr_->getQueue(), imageMgr.getImage(), mapSpan.data(), 0, nullptr, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    return {};
}

std::expected<cl_ulong, Error> CommandBufferManager::getDispatchElapsedTimeNs() const noexcept {
    cl_int clErr;

    cl_ulong time_start;
    cl_ulong time_end;

    clErr = clGetEventProfilingInfo(dispatchEv_, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    clErr = clGetEventProfilingInfo(dispatchEv_, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    cl_ulong timecostNs = time_end - time_start;
    return timecostNs;
}

}  // namespace clc
