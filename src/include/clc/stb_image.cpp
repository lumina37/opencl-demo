#include <cstddef>
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

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/stb_image.hpp"
#endif

namespace clc {

namespace fs = std::filesystem;

StbImageManager::StbImageManager(const fs::path& path) {
    int width, height, oriComps;
    constexpr int comps = 4;
    image_ = (std::byte*)stbi_load(path.string().c_str(), &width, &height, &oriComps, comps);
    extent_ = {width, height, mapStbCompsToClChannelOrder(comps)};
}

StbImageManager::StbImageManager(const Extent& extent) : extent_(extent) {
    image_ = (std::byte*)STBI_MALLOC(extent.size());
}

StbImageManager::~StbImageManager() noexcept { STBI_FREE(image_); }

void StbImageManager::saveTo(const fs::path& path) const {
    stbi_write_png(path.string().c_str(), extent_.width(), extent_.height(), extent_.bpp(), image_,
                   (int)extent_.rowPitch());
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
