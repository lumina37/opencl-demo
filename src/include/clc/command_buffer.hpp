#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include <CL/cl.h>

#include "clc/device/queue.hpp"
#include "clc/extent.hpp"
#include "clc/kernel.hpp"
#include "clc/resource.hpp"

namespace clc {

struct GroupSize {
    size_t x, y;
};

class CommandBufferManager {
public:
    CommandBufferManager(const std::shared_ptr<QueueManager>& pQueueMgr);
    ~CommandBufferManager();

    void uploadBufferFrom(BufferManager& dstBufferMgr, std::span<std::byte> src);
    void uploadImageFrom(ImageManager& dstImageMgr, std::span<std::byte> src, Extent extent);
    void dispatch(const KernelManager& kernelMgr, Extent extent, GroupSize localGroupSize);
    void downloadImageTo(const ImageManager& srcImageMgr, std::span<std::byte> dst, Extent extent);
    void waitDownloadComplete();

    std::span<std::byte> mmapForHostRead(ImageViewManager& imageViewMgr, Extent extent);
    void unmap(ImageViewManager& imageViewMgr, std::span<std::byte> mapSpan);

    [[nodiscard]] cl_ulong getDispatchElapsedTimeNs() const;

private:
    std::shared_ptr<QueueManager> pQueueMgr_;
    std::vector<cl_event> uploadEvs_;
    std::vector<cl_event> downloadEvs_;
    cl_event dispatchEv_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/command_buffer.cpp"
#endif
