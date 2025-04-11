#pragma once

#include <CL/cl.h>

namespace clc {

[[nodiscard]] int mapClChannelOrderToBpp(cl_channel_order chanOrder);

}

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/helper/format.cpp"
#endif
