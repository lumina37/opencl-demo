#include <utility>

#include <CL/cl.h>

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

}  // namespace clc
