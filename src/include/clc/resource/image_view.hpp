#pragma once

#include <cstddef>
#include <span>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/kernel/structs.hpp"
#include "clc/resource/type.hpp"

namespace clc {

class ImageViewManager {
    ImageViewManager(cl_mem&& image) noexcept;

public:
    ImageViewManager(ImageViewManager&& rhs) noexcept;
    ~ImageViewManager() noexcept;

    [[nodiscard]] static std::expected<ImageViewManager, cl_int> create(ContextManager& contextMgr, Extent extent,
                                                                        ResourceType type,
                                                                        std::span<std::byte> hostMem) noexcept;

    [[nodiscard]] cl_mem getImage() const noexcept { return image_; }
    [[nodiscard]] KernelArg genKernelArg() noexcept { return {sizeof(image_), &image_}; }

private:
    cl_mem image_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/image_view.cpp"
#endif
