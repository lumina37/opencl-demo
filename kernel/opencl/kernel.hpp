#pragma once

#include <cstddef>
#include <span>

namespace kernel {

namespace _opencl::grayscale {

#include "opencl/grayscale.cl.h"

}

static const std::span grayscaleOclCode{(std::byte*)_opencl::grayscale::source, sizeof(_opencl::grayscale::source)};

}  // namespace shader
