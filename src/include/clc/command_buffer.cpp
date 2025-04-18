#include <array>
#include <cstddef>

#include <CL/cl.h>

#include "clc/device/queue.hpp"
#include "clc/extent.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/command_buffer.hpp"
#endif

namespace clc {

CommandBufferManager::CommandBufferManager(std::shared_ptr<QueueManager>&& pQueueMgr) noexcept
    : pQueueMgr_(pQueueMgr) {}

std::expected<CommandBufferManager, cl_int> CommandBufferManager::create(
    const std::shared_ptr<QueueManager>& pQueueMgr) noexcept {
    auto copiedPQueueMgr = pQueueMgr;
    return CommandBufferManager{std::move(copiedPQueueMgr)};
}

std::expected<void, cl_int> CommandBufferManager::uploadBufferFrom(BufferManager& dstBufferMgr,
                                                                   const std::span<std::byte> src) noexcept {
    preEvs_.emplace_back();
    const cl_int clErr = clEnqueueWriteBuffer(pQueueMgr_->getQueue(), dstBufferMgr.getBuffer(), false, 0, src.size(),
                                              src.data(), 0, nullptr, &preEvs_.back());

    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    return {};
}

std::expected<void, cl_int> CommandBufferManager::uploadImageFrom(ImageManager& dstImageMgr,
                                                                  const std::span<std::byte> src,
                                                                  const Extent extent) noexcept {
    preEvs_.emplace_back();
    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    const cl_int clErr =
        clEnqueueWriteImage(pQueueMgr_->getQueue(), dstImageMgr.getImage(), false, origin.data(), region.data(),
                            extent.rowPitch(), 0, src.data(), 0, nullptr, &preEvs_.back());

    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    return {};
}

std::expected<void, cl_int> CommandBufferManager::dispatch(const KernelManager& kernelMgr, Extent extent,
                                                           GroupSize localGroupSize) noexcept {
    const GroupSize globalGroupSize{(extent.width() + localGroupSize.x - 1) / localGroupSize.x * localGroupSize.x,
                                    (extent.height() + localGroupSize.y - 1) / localGroupSize.y * localGroupSize.y};
    const cl_int clErr =
        clEnqueueNDRangeKernel(pQueueMgr_->getQueue(), kernelMgr.getKernel(), 2, nullptr, (size_t*)&globalGroupSize,
                               (size_t*)&localGroupSize, preEvs_.size(), preEvs_.data(), &dispatchEv_);

    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    return {};
}

std::expected<void, cl_int> CommandBufferManager::downloadImageTo(const ImageManager& srcImageMgr,
                                                                  std::span<std::byte> dst,
                                                                  const Extent extent) noexcept {
    postEvs_.emplace_back();
    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    const cl_int clErr =
        clEnqueueReadImage(pQueueMgr_->getQueue(), srcImageMgr.getImage(), false, origin.data(), region.data(),
                           extent.rowPitch(), 0, dst.data(), 1, &dispatchEv_, &postEvs_.back());

    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    return {};
}

std::expected<void, cl_int> CommandBufferManager::waitTransferComplete() noexcept {
    const cl_int clErr = clWaitForEvents(postEvs_.size(), postEvs_.data());
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    return {};
}

std::expected<std::span<std::byte>, cl_int> CommandBufferManager::mmapForHostRead(ImageViewManager& imageViewMgr,
                                                                                  const Extent extent) noexcept {
    cl_int clErr;

    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    size_t rowPitch;
    const void* mapPtr =
        clEnqueueMapImage(pQueueMgr_->getQueue(), imageViewMgr.getImage(), true, CL_MAP_READ, origin.data(),
                          region.data(), &rowPitch, nullptr, 1, &dispatchEv_, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    const std::span mapSpan{(std::byte*)mapPtr, extent.size()};
    return mapSpan;
}

std::expected<void, cl_int> CommandBufferManager::unmap(ImageViewManager& imageViewMgr,
                                                        const std::span<std::byte> mapSpan) noexcept {
    const cl_int clErr =
        clEnqueueUnmapMemObject(pQueueMgr_->getQueue(), imageViewMgr.getImage(), mapSpan.data(), 0, nullptr, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    return {};
}

std::expected<cl_ulong, cl_int> CommandBufferManager::getDispatchElapsedTimeNs() const noexcept {
    cl_int clErr;

    cl_ulong time_start;
    cl_ulong time_end;

    clErr = clGetEventProfilingInfo(dispatchEv_, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};
    clErr = clGetEventProfilingInfo(dispatchEv_, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    cl_ulong timecostNs = time_end - time_start;
    return timecostNs;
}

}  // namespace clc
