#include <expected>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/helper/error.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/image.hpp"
#endif

namespace clc {

ImageManager::ImageManager(cl_mem image) noexcept : image_(image) {}

ImageManager::ImageManager(ImageManager&& rhs) noexcept { image_ = std::exchange(rhs.image_, nullptr); }

ImageManager::~ImageManager() noexcept {
    if (image_ == nullptr) return;
    clReleaseMemObject(image_);
    image_ = nullptr;
}

std::expected<ImageManager, Error> ImageManager::create(ContextManager& contextMgr, const Extent extent,
                                                        const cl_mem_flags memType) noexcept {
    cl_int clErr;

    cl_image_desc imageDesc{};
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = extent.width();
    imageDesc.image_height = extent.height();

    cl_image_format imageFormat;
    imageFormat.image_channel_order = extent.clChannelOrder();
    imageFormat.image_channel_data_type = extent.clChannelType();

    auto context = contextMgr.getContext();
    cl_mem image = clCreateImage(context, memType, &imageFormat, &imageDesc, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return ImageManager{image};
}

std::expected<ImageManager, Error> ImageManager::createWrite(ContextManager& contextMgr, const Extent extent) noexcept {
    return create(contextMgr, extent, CL_MEM_WRITE_ONLY);
}

std::expected<ImageManager, Error> ImageManager::createRead(ContextManager& contextMgr, const Extent extent) noexcept {
    return create(contextMgr, extent, CL_MEM_READ_ONLY);
}

std::expected<ImageManager, Error> ImageManager::createReadUMA(ContextManager& contextMgr, Extent extent,
                                                               std::span<std::byte> hostMem) noexcept {
    cl_int clErr;

    auto context = contextMgr.getContext();
    constexpr cl_mem_flags memType = CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY;

    cl_image_desc imageDesc{};
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = extent.width();
    imageDesc.image_height = extent.height();
    imageDesc.image_row_pitch = extent.rowPitch();

    cl_image_format imageFormat;
    imageFormat.image_channel_order = extent.clChannelOrder();
    imageFormat.image_channel_data_type = extent.clChannelType();

    cl_mem image = clCreateImage(context, memType, &imageFormat, &imageDesc, hostMem.data(), &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return ImageManager{image};
}

std::expected<ImageManager, Error> ImageManager::createWriteUMA(ContextManager& contextMgr, Extent extent) noexcept {
    cl_int clErr;

    auto context = contextMgr.getContext();
    constexpr cl_mem_flags memType = CL_MEM_ALLOC_HOST_PTR | CL_MEM_WRITE_ONLY;

    cl_image_desc imageDesc{};
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = extent.width();
    imageDesc.image_height = extent.height();

    cl_image_format imageFormat;
    imageFormat.image_channel_order = extent.clChannelOrder();
    imageFormat.image_channel_data_type = extent.clChannelType();

    cl_mem image = clCreateImage(context, memType, &imageFormat, &imageDesc, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return ImageManager{image};
}

}  // namespace clc
