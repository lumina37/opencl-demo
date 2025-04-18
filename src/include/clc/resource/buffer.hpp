#pragma once

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/kernel/structs.hpp"
#include "clc/resource/type.hpp"

namespace clc {

class BufferManager {
    BufferManager(cl_mem&& buffer) noexcept;

public:
    BufferManager(ContextManager& contextMgr, size_t size, ResourceType type);
    ~BufferManager() noexcept;

    [[nodiscard]] static std::expected<BufferManager, cl_int> create(ContextManager& contextMgr, size_t size,
                                                                     ResourceType type) noexcept;

    [[nodiscard]] cl_mem getBuffer() const noexcept { return buffer_; }
    [[nodiscard]] KernelArg genKernelArg() noexcept { return {sizeof(buffer_), &buffer_}; }

private:
    cl_mem buffer_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/buffer.cpp"
#endif
