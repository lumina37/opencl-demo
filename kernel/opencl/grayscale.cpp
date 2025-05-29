#include <cstddef>
#include <span>

#include "opencl/grayscale.hpp"

namespace kernel::grayscale {

namespace fp32 {

namespace _detail {
#include "grayscale/fp32.h"
}

const std::span<std::byte> code{(std::byte*)_detail::code, sizeof(_detail::code)};

}  // namespace fp32

namespace fp16 {

namespace _detail {
#include "grayscale/fp16.h"
}

const std::span<std::byte> code{(std::byte*)_detail::code, sizeof(_detail::code)};

}  // namespace fp16

}  // namespace kernel::grayscale
