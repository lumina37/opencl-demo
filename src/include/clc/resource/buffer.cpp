#include <cstddef>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/helper/exception.hpp"
#include "clc/resource/type.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/buffer.hpp"
#endif

namespace clc {

BufferManager::BufferManager(ContextManager& contextMgr, const size_t size, const ResourceType type) {
    cl_int errCode;

    auto context = contextMgr.getContext();
    int clBufferType = type == ResourceType::Read ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY;
    buffer_ = clCreateBuffer(context, clBufferType, size, nullptr, &errCode);
    checkError(errCode);
}

BufferManager::~BufferManager() { clReleaseMemObject(buffer_); }

}  // namespace clc
