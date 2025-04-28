#include <cstddef>
#include <expected>
#include <iostream>
#include <print>
#include <ranges>
#include <utility>

#include <CL/cl.h>

#include "clc/common/defines.hpp"
#include "clc/device/manager.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/kernel/manager.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

KernelManager::KernelManager(cl_program&& program, cl_kernel&& kernel) noexcept : program_(program), kernel_(kernel) {}

KernelManager::KernelManager(KernelManager&& rhs) noexcept {
    program_ = std::exchange(rhs.program_, nullptr);
    kernel_ = std::exchange(rhs.kernel_, nullptr);
}

KernelManager::~KernelManager() noexcept {
    if (kernel_ != nullptr) {
        clReleaseKernel(kernel_);
        kernel_ = nullptr;
    }
    if (program_ != nullptr) {
        clReleaseProgram(program_);
        program_ = nullptr;
    }
}
std::expected<KernelManager, Error> KernelManager::create(DeviceManager& deviceMgr, ContextManager& contextMgr,
                                                          std::span<std::byte> code) noexcept {
    cl_int clErr;

    auto context = contextMgr.getContext();
    auto pCode = (const char*)code.data();
    cl_program program = clCreateProgramWithSource(context, 1, &pCode, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    auto device = deviceMgr.getDevice();
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

    return KernelManager{std::move(program), std::move(kernel)};
}

std::expected<void, Error> KernelManager::setKernelArgs(const std::span<KernelArg> args) noexcept {
    for (const auto& [idx, arg] : rgs::views::enumerate(args)) {
        cl_int clErr = clSetKernelArg(kernel_, idx, arg.size, arg.ptr);
        if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};
    }
    return {};
}

}  // namespace clc
