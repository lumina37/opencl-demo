#pragma once

#include <cstddef>
#include <expected>
#include <span>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/kernel/structs.hpp"

namespace clc {

class ImageBox {
    ImageBox(cl_mem image) noexcept;

public:
    ImageBox(const ImageBox&) noexcept = delete;
    ImageBox& operator=(const ImageBox&) noexcept = delete;
    ImageBox(ImageBox&& rhs) noexcept;
    ImageBox& operator=(ImageBox&& rhs) noexcept;
    ~ImageBox() noexcept;

    [[nodiscard]] static std::expected<ImageBox, Error> create(ContextBox& contextBox, Extent extent,
                                                               cl_mem_flags memType) noexcept;
    [[nodiscard]] static std::expected<ImageBox, Error> createRead(ContextBox& contextBox, Extent extent) noexcept;
    [[nodiscard]] static std::expected<ImageBox, Error> createWrite(ContextBox& contextBox, Extent extent) noexcept;
    [[nodiscard]] static std::expected<ImageBox, Error> createReadUMA(ContextBox& contextBox, Extent extent,
                                                                      std::span<std::byte> hostMem) noexcept;
    [[nodiscard]] static std::expected<ImageBox, Error> createWriteUMA(ContextBox& contextBox, Extent extent) noexcept;

    [[nodiscard]] cl_mem getImage() const noexcept { return image_; }
    [[nodiscard]] KernelArg genKernelArg() noexcept { return {sizeof(image_), &image_}; }

private:
    cl_mem image_;
};

}  // namespace clc

#ifdef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/image.cpp"
#endif
