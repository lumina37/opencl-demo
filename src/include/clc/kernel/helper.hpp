#pragma once

#include "clc/kernel/concepts.hpp"

namespace clc {

template <typename... TBox>
[[nodiscard]] static constexpr auto genKernelArgs(TBox&... boxes) noexcept {
    const auto genKernelArg = []<typename T>(T& box) {
        if constexpr (CSupportGenKernelArg<T>) {
            return box.genKernelArg();
        } else {
            return KernelArg{sizeof(T), &box};
        }
    };

    return std::array{genKernelArg(boxes)...};
}

}  // namespace clc
