#include "clc/helper/math.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/extent.hpp"
#endif

namespace clc {

Extent& Extent::setRowAlign(const int rowAlign) noexcept {
    rowAlign_ = rowAlign;
    rowPitch_ = alignUp(width_ * bpp_, rowAlign);
    size_ = rowPitch_ * height_;
    return *this;
}

}  // namespace clc
