#pragma once

#include <cstddef>
#include <expected>
#include <span>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/kernel/structs.hpp"

namespace clc {

class ImageManager {
    ImageManager(cl_mem&& image) noexcept;

public:
    ImageManager(ImageManager&& rhs) noexcept;
    ~ImageManager() noexcept;

    [[nodiscard]] static std::expected<ImageManager, Error> create(ContextManager& contextMgr, Extent extent,
                                                                    cl_mem_flags memType) noexcept;
    [[nodiscard]] static std::expected<ImageManager, Error> createRead(ContextManager& contextMgr,
                                                                        Extent extent) noexcept;
    [[nodiscard]] static std::expected<ImageManager, Error> createWrite(ContextManager& contextMgr,
                                                                         Extent extent) noexcept;
    [[nodiscard]] static std::expected<ImageManager, Error> createReadUMA(ContextManager& contextMgr, Extent extent,
                                                                           std::span<std::byte> hostMem) noexcept;
    [[nodiscard]] static std::expected<ImageManager, Error> createWriteUMA(ContextManager& contextMgr,
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
