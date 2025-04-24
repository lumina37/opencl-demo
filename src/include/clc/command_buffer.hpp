#pragma once

#include <cstddef>
#include <expected>
#include <memory>
#include <vector>

#include <CL/cl.h>

#include "clc/device/queue.hpp"
#include "clc/extent.hpp"
#include "clc/helper/error.hpp"
#include "clc/kernel.hpp"
#include "clc/resource.hpp"

namespace clc {

struct GroupSize {
    size_t x, y;
};

class CommandBufferManager {
    CommandBufferManager(std::shared_ptr<QueueManager>&& pQueueMgr) noexcept;

public:
    [[nodiscard]] static std::expected<CommandBufferManager, Error> create(
        const std::shared_ptr<QueueManager>& pQueueMgr) noexcept;

    [[nodiscard]] std::expected<void, Error> uploadBufferFrom(BufferManager& dstBufferMgr,
                                                               std::span<std::byte> src) noexcept;
    [[nodiscard]] std::expected<void, Error> uploadImageFrom(ImageManager& dstImageMgr, std::span<std::byte> src,
                                                              Extent extent) noexcept;
    [[nodiscard]] std::expected<void, Error> dispatch(const KernelManager& kernelMgr, Extent extent,
                                                      GroupSize localGroupSize) noexcept;
    [[nodiscard]] std::expected<void, Error> downloadImageTo(const ImageManager& srcImageMgr, std::span<std::byte> dst,
                                                              Extent extent) noexcept;
    [[nodiscard]] std::expected<void, Error> waitTransferComplete() noexcept;

    [[nodiscard]] std::expected<std::span<std::byte>, Error> mmapForHostRead(ImageManager& imageMgr,
                                                                              Extent extent) noexcept;
    [[nodiscard]] std::expected<std::span<std::byte>, Error> mmapForHostWrite(ImageManager& imageMgr,
                                                                               Extent extent) noexcept;
    [[nodiscard]] std::expected<void, Error> unmap(ImageManager& imageMgr, std::span<std::byte> mapSpan) noexcept;

    [[nodiscard]] std::expected<cl_ulong, Error> getDispatchElapsedTimeNs() const noexcept;

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
