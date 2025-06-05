#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <span>
#include <utility>

#include "clc/extent.hpp"
#include "clc/helper/error.hpp"

namespace clc {

namespace fs = std::filesystem;

class StbImageBox {
    StbImageBox(std::byte* image, Extent extent) noexcept;

public:
    StbImageBox(StbImageBox&& rhs) noexcept;
    ~StbImageBox() noexcept;

    [[nodiscard]] static std::expected<StbImageBox, Error> createFromPath(const fs::path& path) noexcept;
    [[nodiscard]] static std::expected<StbImageBox, Error> createWithExtent(Extent extent) noexcept;

    [[nodiscard]] std::span<std::byte> getImageSpan() const noexcept { return {image_, extent_.size()}; }

    template <typename Self>
    [[nodiscard]] auto&& getExtent(this Self&& self) noexcept {
        return std::forward_like<Self>(self).extent_;
    }

    [[nodiscard]] std::expected<void, Error> saveTo(const fs::path& path) const noexcept;

    static constexpr cl_channel_order mapStbCompsToClChannelOrder(int comps) noexcept;

private:
    std::byte* image_;
    Extent extent_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/stb_image.cpp"
#endif
