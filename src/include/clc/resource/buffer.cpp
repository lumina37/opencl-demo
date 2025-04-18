#include <expected>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/resource/type.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/buffer.hpp"
#endif

namespace clc {

BufferManager::BufferManager(cl_mem&& buffer) noexcept : buffer_(buffer) {}

BufferManager::~BufferManager() noexcept {
    if (buffer_ == nullptr) return;
    clReleaseMemObject(buffer_);
    buffer_ = nullptr;
}

std::expected<BufferManager, cl_int> BufferManager::create(ContextManager& contextMgr, size_t size,
                                                           ResourceType type) noexcept {
    cl_int clErr;

    auto context = contextMgr.getContext();
    int clBufferType = type == ResourceType::Read ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY;
    cl_mem buffer = clCreateBuffer(context, clBufferType, size, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    return BufferManager{std::move(buffer)};
}

}  // namespace clc
