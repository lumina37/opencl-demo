#include <expected>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/buffer.hpp"
#endif

namespace clc {

BufferManager::BufferManager(cl_mem&& buffer) noexcept : buffer_(buffer) {}

BufferManager::BufferManager(BufferManager&& rhs) noexcept : buffer_(std::exchange(rhs.buffer_, nullptr)) {}

BufferManager::~BufferManager() noexcept {
    if (buffer_ == nullptr) return;
    clReleaseMemObject(buffer_);
    buffer_ = nullptr;
}

std::expected<BufferManager, cl_int> BufferManager::create(ContextManager& contextMgr, size_t size,
                                                           const cl_mem_flags memType) noexcept {
    cl_int clErr;

    auto context = contextMgr.getContext();
    cl_mem buffer = clCreateBuffer(context, memType, size, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    return BufferManager{std::move(buffer)};
}

std::expected<BufferManager, cl_int> BufferManager::createRead(ContextManager& contextMgr, size_t size) noexcept {
    return create(contextMgr, size, CL_MEM_READ_ONLY);
}

std::expected<BufferManager, cl_int> BufferManager::createWrite(ContextManager& contextMgr, size_t size) noexcept {
    return create(contextMgr, size, CL_MEM_WRITE_ONLY);
}

}  // namespace clc
