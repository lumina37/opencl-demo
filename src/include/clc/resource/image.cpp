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

ImageBox::ImageBox(cl_mem image) noexcept : image_(image) {}

ImageBox::ImageBox(ImageBox&& rhs) noexcept { image_ = std::exchange(rhs.image_, nullptr); }

ImageBox::~ImageBox() noexcept {
    if (image_ == nullptr) return;
    clReleaseMemObject(image_);
    image_ = nullptr;
}

std::expected<ImageBox, Error> ImageBox::create(ContextBox& contextBox, const Extent extent,
                                                const cl_mem_flags memType) noexcept {
    cl_int clErr;

    cl_image_desc imageDesc{};
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = extent.width();
    imageDesc.image_height = extent.height();

    cl_image_format imageFormat;
    imageFormat.image_channel_order = extent.clChannelOrder();
    imageFormat.image_channel_data_type = extent.clChannelType();

    auto context = contextBox.getContext();
    cl_mem image = clCreateImage(context, memType, &imageFormat, &imageDesc, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{Error{clErr}};

    return ImageBox{image};
}

std::expected<ImageBox, Error> ImageBox::createWrite(ContextBox& contextBox, const Extent extent) noexcept {
    return create(contextBox, extent, CL_MEM_WRITE_ONLY);
}

std::expected<ImageBox, Error> ImageBox::createRead(ContextBox& contextBox, const Extent extent) noexcept {
    return create(contextBox, extent, CL_MEM_READ_ONLY);
}

std::expected<ImageBox, Error> ImageBox::createReadUMA(ContextBox& contextBox, Extent extent,
                                                       std::span<std::byte> hostMem) noexcept {
    cl_int clErr;

    auto context = contextBox.getContext();
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

    return ImageBox{image};
}

std::expected<ImageBox, Error> ImageBox::createWriteUMA(ContextBox& contextBox, Extent extent) noexcept {
    cl_int clErr;

    auto context = contextBox.getContext();
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

    return ImageBox{image};
}

}  // namespace clc
