#pragma once

#include <cstddef>
#include <span>

namespace kernel::grayscale {

namespace fp32 {

extern const std::span<std::byte> code;

}  // namespace fp32

namespace fp16 {

extern const std::span<std::byte> code;

}  // namespace fp16

}  // namespace kernel::grayscale
