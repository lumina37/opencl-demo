#include <cstddef>
#include <iostream>
#include <print>
#include <ranges>

#include <CL/cl.h>

#include "clc/device/device.hpp"
#include "clc/helper/defines.hpp"
#include "clc/helper/exception.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/kernel.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

KernelManager::KernelManager(DeviceManager& deviceMgr, ContextManager& contextMgr, const std::span<std::byte> code) {
    cl_int errCode;

    auto context = contextMgr.getContext();
    auto pCode = (const char*)code.data();
    program_ = clCreateProgramWithSource(context, 1, &pCode, nullptr, &errCode);
    checkError(errCode);

    auto device = deviceMgr.getDevice();
    errCode = clBuildProgram(program_, 1, &device, nullptr, nullptr, nullptr);

    if constexpr (ENABLE_DEBUG) {
        if (errCode != CL_SUCCESS) {
            size_t logSize;
            clGetProgramBuildInfo(program_, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
            auto pLog = std::make_unique_for_overwrite<char[]>(logSize);
            clGetProgramBuildInfo(program_, device, CL_PROGRAM_BUILD_LOG, logSize, pLog.get(), nullptr);
            std::println(std::cerr, "Kernel build failed: {}", pLog.get());
        }
    }

    checkError(errCode);
    kernel_ = clCreateKernel(program_, "clcmain", &errCode);
    checkError(errCode);
}

KernelManager::~KernelManager() {
    clReleaseKernel(kernel_);
    clReleaseProgram(program_);
}

void KernelManager::setKernelArgs(const std::span<KernelArg> args) {
    for (auto [idx, arg] : rgs::views::enumerate(args)) {
        cl_int errCode = clSetKernelArg(kernel_, idx, arg.size, arg.ptr);
        checkError(errCode);
    }
}

}  // namespace clc
