#pragma once

#include <utility>

#include <CL/cl.h>

namespace clc {

[[nodiscard]] constexpr int mapClFormatToBpp(const cl_channel_order chanOrder,
                                             const cl_channel_type chanType) noexcept {
    int comps;
    switch (chanOrder) {
        case CL_R:
            comps = 1;
            break;
        case CL_A:
            comps = 1;
            break;
        case CL_RG:
            comps = 2;
            break;
        case CL_RGB:
            comps = 3;
            break;
        case CL_RGBA:
            comps = 4;
            break;
        default:
            std::unreachable();
    }

    int bpc;
    switch (chanType) {
        case CL_UNORM_INT8:
            bpc = 1;
            break;
        case CL_FLOAT:
            bpc = 4;
            break;
        default:
            std::unreachable();
    }

    const int bpp = comps * bpc;
    return bpp;
}

}  // namespace clc
