#pragma once

#include <compare>
#include <cstdint>

namespace clc {

template <typename TAttach>
class Score {
public:
    int64_t score;
    TAttach attachment;

    static friend constexpr std::weak_ordering operator<=>(const Score& lhs, const Score& rhs) noexcept {
        return lhs.score <=> rhs.score;
    }
};

}  // namespace clc
