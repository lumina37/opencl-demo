#pragma once

#include <cstddef>
#include <expected>
#include <span>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/kernel/structs.hpp"
#include "clc/resource/type.hpp"

namespace clc {

class ImageManager {
    ImageManager(cl_mem&& image) noexcept;

public:
    ImageManager(ImageManager&& rhs) noexcept;
    ~ImageManager() noexcept;

    [[nodiscard]] static std::expected<ImageManager, cl_int> create(ContextManager& contextMgr, Extent extent,
                                                                    ResourceType type) noexcept;
    [[nodiscard]] static std::expected<ImageManager, cl_int> createUmaRead(ContextManager& contextMgr, Extent extent,
                                                                           std::span<std::byte> hostMem) noexcept;
    [[nodiscard]] static std::expected<ImageManager, cl_int> createUmaWrite(ContextManager& contextMgr,
                                                                            Extent extent) noexcept;

    [[nodiscard]] cl_mem getImage() const noexcept { return image_; }
    [[nodiscard]] KernelArg genKernelArg() noexcept { return {sizeof(image_), &image_}; }

private:
    cl_mem image_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/image.cpp"
#endif
