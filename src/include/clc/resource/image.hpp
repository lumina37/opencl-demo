#pragma once

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/kernel/structs.hpp"
#include "clc/resource/type.hpp"

namespace clc {

class ImageManager {
public:
    ImageManager(ContextManager& contextMgr, Extent extent, ResourceType type);
    ~ImageManager();

    [[nodiscard]] cl_mem getImage() const noexcept { return image_; }
    [[nodiscard]] KernelArg genKernelArg() noexcept { return {sizeof(image_), &image_}; }

private:
    cl_mem image_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/image.cpp"
#endif
