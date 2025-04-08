#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <utility>

#include "clc/extent.hpp"

namespace clc {

namespace fs = std::filesystem;

class StbImageManager {
public:
    StbImageManager(const fs::path& path);
    StbImageManager(const Extent& extent);
    ~StbImageManager() noexcept;

    std::span<std::byte> getImageSpan() const noexcept { return {image_, extent_.size()}; }

    template <typename Self>
    [[nodiscard]] auto&& getExtent(this Self&& self) noexcept {
        return std::forward_like<Self>(self).extent_;
    }

    void saveTo(const fs::path& path) const;

private:
    std::byte* image_;
    Extent extent_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/stb_image.cpp"
#endif
