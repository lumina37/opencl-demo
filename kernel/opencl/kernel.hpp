#pragma once

#include <cstddef>
#include <span>

namespace kernel {

namespace _opencl::gray_fp32 {

#include "opencl/grayscale_fp32.h"

}

namespace _opencl::gray_fp16 {

#include "opencl/grayscale_fp16.h"

}

static const std::span grayscaleFp32Code{(std::byte*)_opencl::gray_fp32::source, sizeof(_opencl::gray_fp32::source)};
static const std::span grayscaleFp16Code{(std::byte*)_opencl::gray_fp16::source, sizeof(_opencl::gray_fp16::source)};

}  // namespace kernel
