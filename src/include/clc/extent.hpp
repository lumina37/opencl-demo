#pragma once

#include <cstddef>

#include <CL/cl.h>

namespace clc {

class Extent {
public:
    Extent() = default;
    Extent(const int width, const int height, const int comps)
        : width_(width), height_(height), comps_(comps), size_(width * height * comps) {}

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    [[nodiscard]] size_t comps() const noexcept { return comps_; }
    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] cl_channel_order clChannel() const noexcept;

private:
    int width_;
    int height_;
    size_t comps_;
    size_t size_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/extent.cpp"
#endif
