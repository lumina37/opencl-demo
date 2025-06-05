#include <expected>
#include <utility>

#include <CL/cl.h>

#include "clc/device/box.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/context.hpp"
#endif

namespace clc {

ContextBox::ContextBox(cl_context context) noexcept : context_(context) {}

ContextBox::ContextBox(ContextBox&& rhs) noexcept { context_ = std::exchange(rhs.context_, nullptr); }

ContextBox::~ContextBox() noexcept {
    if (context_ == nullptr) return;
    clReleaseContext(context_);
    context_ = nullptr;
}

std::expected<ContextBox, Error> ContextBox::create(DeviceBox& deviceBox) noexcept {
    cl_int clErr;

    auto device = deviceBox.getDevice();
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return ContextBox{context};
}

}  // namespace clc
