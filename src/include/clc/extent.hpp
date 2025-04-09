#pragma once

#include <cstddef>

#include <CL/cl.h>

#include "clc/helper/math.hpp"

namespace clc {

class Extent {
public:
    Extent() = default;
    Extent(const int width, const int height, const int comps)
        : width_(width),
          height_(height),
          comps_(comps),
          rowAlign_(1),
          rowPitch_(width * comps),
          size_(rowPitch_ * height) {}
    Extent(const int width, const int height, const int comps, const int rowAlign)
        : width_(width),
          height_(height),
          comps_(comps),
          rowAlign_(rowAlign),
          rowPitch_(alignUp(width * comps, rowAlign)),
          size_(rowPitch_ * height) {}

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    [[nodiscard]] int comps() const noexcept { return comps_; }
    [[nodiscard]] int rowAlign() const noexcept { return rowAlign_; }
    [[nodiscard]] size_t rowPitch() const noexcept { return rowPitch_; }
    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] cl_channel_order clChannel() const noexcept;

    Extent& setRowAlign(int rowAlign) noexcept;

private:
    int width_;
    int height_;
    int comps_;
    int rowAlign_;
    size_t rowPitch_;
    size_t size_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/extent.cpp"
#endif
