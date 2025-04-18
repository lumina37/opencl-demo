#pragma once

#include <type_traits>
#include <utility>

#include "clc/kernel/concepts.hpp"

namespace clc {

template <typename... Tv>
    requires((CSupportGenKernelArg<Tv> || std::is_trivially_copyable_v<Tv>) && ...)
[[nodiscard]] static constexpr inline auto genKernelArgs(Tv&... mgrs) noexcept {
    const auto genKernelArg = []<typename T>(T& mgr) {
        if constexpr (CSupportGenKernelArg<T>) {
            return mgr.genKernelArg();
        } else if constexpr (std::is_trivially_copyable_v<T>) {
            return KernelArg{sizeof(T), &mgr};
        } else {
            std::unreachable();
        }
    };

    return std::array{genKernelArg(mgrs)...};
}

}  // namespace clc
