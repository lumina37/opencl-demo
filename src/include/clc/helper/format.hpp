#pragma once

#include <CL/cl.h>

namespace clc {

[[nodiscard]] constexpr int mapClChannelOrderToBpp(cl_channel_order chanOrder) noexcept {
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
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/helper/format.cpp"
#endif
