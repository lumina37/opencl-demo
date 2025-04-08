#include <array>
#include <cstddef>

#include <CL/cl.h>

#include "clc/device/queue.hpp"
#include "clc/extent.hpp"
#include "clc/helper/exception.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/command_buffer.hpp"
#endif

namespace clc {

CommandBufferManager::CommandBufferManager(const std::shared_ptr<QueueManager>& pQueueMgr) : pQueueMgr_(pQueueMgr) {}

CommandBufferManager::~CommandBufferManager() {}

void CommandBufferManager::uploadImageFrom(ImageManager& dstImageMgr, const std::span<std::byte> src,
                                           const Extent extent) {
    cl_int errCode;

    uploadEvs_.emplace_back();
    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    errCode = clEnqueueWriteImage(pQueueMgr_->getQueue(), dstImageMgr.getImage(), false, origin.data(), region.data(),
                                  0, 0, src.data(), 0, nullptr, &uploadEvs_.back());
    checkError(errCode);
}

void CommandBufferManager::dispatch(const KernelManager& kernelMgr, Extent extent, GroupSize localGroupSize) {
    cl_int errCode;

    const GroupSize globalGroupSize{(extent.width() + localGroupSize.x - 1) / localGroupSize.x * localGroupSize.x,
                                    (extent.height() + localGroupSize.y - 1) / localGroupSize.y * localGroupSize.y};
    errCode =
        clEnqueueNDRangeKernel(pQueueMgr_->getQueue(), kernelMgr.getKernel(), 2, nullptr, (size_t*)&globalGroupSize,
                               (size_t*)&localGroupSize, uploadEvs_.size(), uploadEvs_.data(), &dispatchEv_);
    checkError(errCode);
}

void CommandBufferManager::downloadImageTo(const ImageManager& srcImageMgr, std::span<std::byte> dst,
                                           const Extent extent) {
    cl_int errCode;

    downloadEvs_.emplace_back();
    const std::array<size_t, 3> origin{0, 0, 0};
    const std::array<size_t, 3> region{(size_t)extent.width(), (size_t)extent.height(), 1};
    errCode = clEnqueueReadImage(pQueueMgr_->getQueue(), srcImageMgr.getImage(), false, origin.data(), region.data(), 0,
                                 0, dst.data(), 1, &dispatchEv_, &downloadEvs_.back());
    checkError(errCode);
}

void CommandBufferManager::waitDownloadComplete() {
    cl_int errCode = clWaitForEvents(downloadEvs_.size(), downloadEvs_.data());
    checkError(errCode);
}

}  // namespace clc
