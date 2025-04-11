#pragma once

#include <utility>

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/helper/format.hpp"
#endif

namespace clc {

int mapClChannelOrderToBpp(const cl_channel_order chanOrder) {
    switch (chanOrder) {
        case CL_R:
            return 1;
        case CL_A:
            return 1;
        case CL_RGB:
            return 3;
        case CL_RGBA:
            return 4;
        default:
            std::unreachable();
    }
}

}  // namespace clc
