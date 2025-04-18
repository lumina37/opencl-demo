#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <span>
#include <utility>

#include "clc/extent.hpp"

namespace clc {

namespace fs = std::filesystem;

class StbImageManager {
public:
    StbImageManager(StbImageManager&& rhs) noexcept;
    ~StbImageManager() noexcept;

    [[nodiscard]] static std::expected<StbImageManager, int> createFromPath(const fs::path& path) noexcept;
    [[nodiscard]] static std::expected<StbImageManager, int> createWithExtent(Extent extent) noexcept;

    std::span<std::byte> getImageSpan() const noexcept { return {image_, extent_.size()}; }

    template <typename Self>
    [[nodiscard]] auto&& getExtent(this Self&& self) noexcept {
        return std::forward_like<Self>(self).extent_;
    }

    std::expected<void, int> saveTo(const fs::path& path) const noexcept;

    static constexpr cl_channel_order mapStbCompsToClChannelOrder(int comps) noexcept;

private:
    StbImageManager(std::byte* image, Extent extent) noexcept;

    std::byte* image_;
    Extent extent_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/stb_image.cpp"
#endif
