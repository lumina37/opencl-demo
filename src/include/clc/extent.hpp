#pragma once

#include <cstddef>

#include <CL/cl.h>

#include "clc/helper/format.hpp"
#include "clc/helper/math.hpp"

namespace clc {

class Extent {
public:
    Extent() = default;
    Extent(const int width, const int height, const cl_channel_order chanOrder, const cl_channel_type chanType)
        : width_(width),
          height_(height),
          chanOrder_(chanOrder),
          chanType_(chanType),
          bpp_(mapClFormatToBpp(chanOrder, chanType)),
          rowAlign_(1),
          rowPitch_(width * bpp_),
          size_(rowPitch_ * height) {}
    Extent(const int width, const int height, const cl_channel_order chanOrder, const cl_channel_type chanType,
           const int rowAlign)
        : width_(width),
          height_(height),
          chanOrder_(chanOrder),
          chanType_(chanType),
          bpp_(mapClFormatToBpp(chanOrder, chanType)),
          rowAlign_(rowAlign),
          rowPitch_(alignUp(width * bpp_, rowAlign)),
          size_(rowPitch_ * height) {}

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    [[nodiscard]] cl_channel_order clChannelOrder() const noexcept { return chanOrder_; }
    [[nodiscard]] cl_channel_type clChannelType() const noexcept { return chanType_; }
    [[nodiscard]] int bpp() const noexcept { return bpp_; }
    [[nodiscard]] int rowAlign() const noexcept { return rowAlign_; }
    [[nodiscard]] size_t rowPitch() const noexcept { return rowPitch_; }
    [[nodiscard]] size_t size() const noexcept { return size_; }

    Extent& setRowAlign(int rowAlign) noexcept;

private:
    int width_;
    int height_;
    cl_channel_order chanOrder_;
    cl_channel_type chanType_;
    int bpp_;
    int rowAlign_;
    size_t rowPitch_;
    size_t size_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/extent.cpp"
#endif
