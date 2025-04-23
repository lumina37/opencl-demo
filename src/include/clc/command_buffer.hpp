#pragma once

#include <cstddef>
#include <expected>
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
    CommandBufferManager(std::shared_ptr<QueueManager>&& pQueueMgr) noexcept;

public:
    [[nodiscard]] static std::expected<CommandBufferManager, cl_int> create(
        const std::shared_ptr<QueueManager>& pQueueMgr) noexcept;

    [[nodiscard]] std::expected<void, cl_int> uploadBufferFrom(BufferManager& dstBufferMgr,
                                                               std::span<std::byte> src) noexcept;
    [[nodiscard]] std::expected<void, cl_int> uploadImageFrom(ImageManager& dstImageMgr, std::span<std::byte> src,
                                                              Extent extent) noexcept;
    [[nodiscard]] std::expected<void, cl_int> dispatch(const KernelManager& kernelMgr, Extent extent,
                                                       GroupSize localGroupSize) noexcept;
    [[nodiscard]] std::expected<void, cl_int> downloadImageTo(const ImageManager& srcImageMgr, std::span<std::byte> dst,
                                                              Extent extent) noexcept;
    [[nodiscard]] std::expected<void, cl_int> waitTransferComplete() noexcept;

    [[nodiscard]] std::expected<std::span<std::byte>, cl_int> mmapForHostRead(ImageManager& imageMgr,
                                                                              Extent extent) noexcept;
    [[nodiscard]] std::expected<std::span<std::byte>, cl_int> mmapForHostWrite(ImageManager& imageMgr,
                                                                               Extent extent) noexcept;
    [[nodiscard]] std::expected<void, cl_int> unmap(ImageManager& imageMgr,
                                                    std::span<std::byte> mapSpan) noexcept;

    [[nodiscard]] std::expected<cl_ulong, cl_int> getDispatchElapsedTimeNs() const noexcept;

private:
    std::shared_ptr<QueueManager> pQueueMgr_;
    std::vector<cl_event> preEvs_;
    std::vector<cl_event> postEvs_;
    cl_event dispatchEv_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/command_buffer.cpp"
#endif
