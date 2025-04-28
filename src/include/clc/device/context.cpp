#include <expected>
#include <utility>

#include <CL/cl.h>

#include "clc/device/manager.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/context.hpp"
#endif

namespace clc {

ContextManager::ContextManager(cl_context&& context) noexcept : context_(context) {}

ContextManager::ContextManager(ContextManager&& rhs) noexcept { context_ = std::exchange(rhs.context_, nullptr); }

ContextManager::~ContextManager() noexcept {
    if (context_ == nullptr) return;
    clReleaseContext(context_);
    context_ = nullptr;
}

std::expected<ContextManager, Error> ContextManager::create(DeviceManager& deviceMgr) noexcept {
    cl_int clErr;

    auto device = deviceMgr.getDevice();
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return ContextManager{std::move(context)};
}

}  // namespace clc
