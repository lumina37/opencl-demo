#include <cstddef>
#include <expected>
#include <filesystem>
#include <utility>

#pragma push_macro("STB_IMAGE_IMPLEMENTATION")
#pragma push_macro("STB_IMAGE_WRITE_IMPLEMENTATION")
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#pragma pop_macro("STB_IMAGE_IMPLEMENTATION")
#pragma pop_macro("STB_IMAGE_WRITE_IMPLEMENTATION")

#include "clc/extent.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/stb_image.hpp"
#endif

namespace clc {

namespace fs = std::filesystem;

StbImageManager::StbImageManager(std::byte* image, Extent extent) noexcept : image_(image), extent_(extent) {}

StbImageManager::StbImageManager(StbImageManager&& rhs) noexcept {
    image_ = std::exchange(rhs.image_, nullptr);
    std::swap(extent_, rhs.extent_);
}

StbImageManager::~StbImageManager() noexcept {
    if (image_ == nullptr) return;
    STBI_FREE(image_);
    image_ = nullptr;
}

std::expected<StbImageManager, Error> StbImageManager::createFromPath(const fs::path& path) noexcept {
    int width, height, oriComps;
    constexpr int comps = 4;

    std::byte* image = (std::byte*)stbi_load(path.string().c_str(), &width, &height, &oriComps, comps);
    if (image == nullptr) return std::unexpected{1};

    Extent extent_{width, height, StbImageManager::mapStbCompsToClChannelOrder(comps), CL_UNORM_INT8};
    return StbImageManager{image, extent_};
}

std::expected<StbImageManager, Error> StbImageManager::createWithExtent(const Extent extent) noexcept {
    std::byte* image = (std::byte*)STBI_MALLOC(extent.size());
    if (image == nullptr) return std::unexpected{1};

    return StbImageManager{image, extent};
}

std::expected<void, Error> StbImageManager::saveTo(const fs::path& path) const noexcept {
    const int stbErr = stbi_write_png(path.string().c_str(), extent_.width(), extent_.height(), extent_.bpp(), image_,
                                      (int)extent_.rowPitch());

    if (stbErr == 0) return std::unexpected{1};
    return {};
}

constexpr cl_channel_order StbImageManager::mapStbCompsToClChannelOrder(const int comps) noexcept {
    switch (comps) {
        case 1:
            return CL_R;
        case 2:
            return CL_RG;
        case 3:
            return CL_RGB;
        case 4:
            return CL_RGBA;
        default:
            std::unreachable();
    }
}

}  // namespace clc
