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

StbImageBox::StbImageBox(std::byte* image, Extent extent) noexcept : image_(image), extent_(extent) {}

StbImageBox::StbImageBox(StbImageBox&& rhs) noexcept {
    image_ = std::exchange(rhs.image_, nullptr);
    std::swap(extent_, rhs.extent_);
}

StbImageBox::~StbImageBox() noexcept {
    if (image_ == nullptr) return;
    STBI_FREE(image_);
    image_ = nullptr;
}

std::expected<StbImageBox, Error> StbImageBox::createFromPath(const fs::path& path) noexcept {
    int width, height, oriComps;
    constexpr int comps = 4;

    std::byte* image = (std::byte*)stbi_load(path.string().c_str(), &width, &height, &oriComps, comps);
    if (image == nullptr) return std::unexpected{Error{1, "Failed to load image"}};

    Extent extent{width, height, StbImageBox::mapStbCompsToClChannelOrder(comps), CL_UNORM_INT8};
    return StbImageBox{image, extent};
}

std::expected<StbImageBox, Error> StbImageBox::createWithExtent(const Extent extent) noexcept {
    std::byte* image = (std::byte*)STBI_MALLOC(extent.size());
    if (image == nullptr) return std::unexpected{Error{1}};

    return StbImageBox{image, extent};
}

std::expected<void, Error> StbImageBox::saveTo(const fs::path& path) const noexcept {
    const int stbErr = stbi_write_png(path.string().c_str(), extent_.width(), extent_.height(), extent_.bpp(), image_,
                                      (int)extent_.rowPitch());

    if (stbErr == 0) return std::unexpected{Error{1, "failed to save image"}};
    return {};
}

constexpr cl_channel_order StbImageBox::mapStbCompsToClChannelOrder(const int comps) noexcept {
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
