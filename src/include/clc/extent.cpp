#include <utility>

#include <CL/cl.h>

#include "clc/helper/math.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/extent.hpp"
#endif

namespace clc {

cl_channel_order Extent::clChannel() const noexcept {
    switch (comps_) {
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

Extent& Extent::setRowAlign(const int rowAlign) noexcept {
    rowAlign_ = rowAlign;
    rowPitch_ = alignUp(width_ * comps_, rowAlign);
    size_ = rowPitch_ * height_;
    return *this;
}

}  // namespace clc
