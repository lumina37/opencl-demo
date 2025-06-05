#include <expected>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/buffer.hpp"
#endif

namespace clc {

BufferBox::BufferBox(cl_mem&& buffer) noexcept : buffer_(buffer) {}

BufferBox::BufferBox(BufferBox&& rhs) noexcept : buffer_(std::exchange(rhs.buffer_, nullptr)) {}

BufferBox::~BufferBox() noexcept {
    if (buffer_ == nullptr) return;
    clReleaseMemObject(buffer_);
    buffer_ = nullptr;
}

std::expected<BufferBox, Error> BufferBox::create(ContextBox& contextBox, size_t size,
                                                          const cl_mem_flags memType) noexcept {
    cl_int clErr;

    auto context = contextBox.getContext();
    cl_mem buffer = clCreateBuffer(context, memType, size, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return BufferBox{std::move(buffer)};
}

std::expected<BufferBox, Error> BufferBox::createRead(ContextBox& contextBox, size_t size) noexcept {
    return create(contextBox, size, CL_MEM_READ_ONLY);
}

std::expected<BufferBox, Error> BufferBox::createWrite(ContextBox& contextBox, size_t size) noexcept {
    return create(contextBox, size, CL_MEM_WRITE_ONLY);
}

}  // namespace clc
