#pragma once

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/kernel/structs.hpp"

namespace clc {

class BufferBox {
    BufferBox(cl_mem buffer) noexcept;

public:
    BufferBox(const BufferBox&) noexcept = delete;
    BufferBox& operator=(const BufferBox&) noexcept = delete;
    BufferBox(BufferBox&& rhs) noexcept;
    BufferBox& operator=(BufferBox&& rhs) noexcept;
    ~BufferBox() noexcept;

    [[nodiscard]] static std::expected<BufferBox, Error> create(ContextBox& contextBox, size_t size,
                                                                cl_mem_flags memType) noexcept;
    [[nodiscard]] static std::expected<BufferBox, Error> createRead(ContextBox& contextBox, size_t size) noexcept;
    [[nodiscard]] static std::expected<BufferBox, Error> createWrite(ContextBox& contextBox, size_t size) noexcept;

    [[nodiscard]] cl_mem getBuffer() const noexcept { return buffer_; }
    [[nodiscard]] KernelArg genKernelArg() noexcept { return {sizeof(buffer_), &buffer_}; }

private:
    cl_mem buffer_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/buffer.cpp"
#endif
