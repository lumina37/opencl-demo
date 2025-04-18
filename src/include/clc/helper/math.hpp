#pragma once

#include <cmath>
#include <concepts>

namespace clc {
template <std::unsigned_integral Tv>
[[nodiscard]] static constexpr inline bool isPowOf2(const Tv v) noexcept {
    return (v & (v - 1)) == 0;
}

template <std::integral Tv>
[[nodiscard]] static constexpr inline Tv alignUp(const Tv v, const size_t to) noexcept {
    return (Tv)(((size_t)v + (to - 1)) & ((~to) + 1));
}

}  // namespace clc
