#include <cstddef>
#include <expected>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <utility>

#include <CL/cl.h>

#include "clc/common/defines.hpp"
#include "clc/device/box.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/kernel/box.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

KernelBox::KernelBox(cl_program program, cl_kernel kernel) noexcept : program_(program), kernel_(kernel) {}

KernelBox::KernelBox(KernelBox&& rhs) noexcept {
    program_ = std::exchange(rhs.program_, nullptr);
    kernel_ = std::exchange(rhs.kernel_, nullptr);
}

KernelBox::~KernelBox() noexcept {
    if (kernel_ != nullptr) {
        clReleaseKernel(kernel_);
        kernel_ = nullptr;
    }
    if (program_ != nullptr) {
        clReleaseProgram(program_);
        program_ = nullptr;
    }
}
std::expected<KernelBox, Error> KernelBox::create(DeviceBox& deviceBox, ContextBox& contextBox,
                                                  std::span<std::byte> code) noexcept {
    cl_int clErr;

    auto context = contextBox.getContext();
    auto pCode = (const char*)code.data();
    cl_program program = clCreateProgramWithSource(context, 1, &pCode, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    auto device = deviceBox.getDevice();
    clErr = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);

    if constexpr (ENABLE_DEBUG) {
        if (clErr != CL_SUCCESS) {
            size_t logSize;
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
            auto pLog = std::make_unique_for_overwrite<char[]>(logSize);
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, pLog.get(), nullptr);
            std::println(std::cerr, "Kernel build failed: {}", pLog.get());
        }
    }
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    cl_kernel kernel = clCreateKernel(program, "clcmain", &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return KernelBox{program, kernel};
}

std::expected<void, Error> KernelBox::setKernelArgs(const std::span<const KernelArg> args) noexcept {
    for (const auto& [idx, arg] : rgs::views::enumerate(args)) {
        cl_int clErr = clSetKernelArg(kernel_, idx, arg.size, arg.ptr);
        if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    }
    return {};
}

}  // namespace clc
