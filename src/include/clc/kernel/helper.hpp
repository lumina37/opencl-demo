#pragma once

#include "clc/kernel/concepts.hpp"

namespace clc {

template <typename... TMgr>
[[nodiscard]] static constexpr auto genKernelArgs(TMgr&... mgrs) noexcept {
    const auto genKernelArg = []<typename T>(T& mgr) {
        if constexpr (CSupportGenKernelArg<T>) {
            return mgr.genKernelArg();
        } else {
            return KernelArg{sizeof(T), &mgr};
        }
    };

    return std::array{genKernelArg(mgrs)...};
}

}  // namespace clc
