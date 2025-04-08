#pragma once

#include <string>

#include <CL/cl.h>

namespace clc {

const std::string_view getClErrorString(cl_int errCode);

void checkError(cl_int errCode);

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/helper/exception.cpp"
#endif
