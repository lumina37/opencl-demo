#include <expected>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/resource/type.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/image.hpp"
#endif

namespace clc {

ImageManager::ImageManager(cl_mem&& image) noexcept : image_(image) {}

ImageManager::ImageManager(ImageManager&& rhs) noexcept { image_ = std::exchange(rhs.image_, nullptr); }

ImageManager::~ImageManager() noexcept {
    if (image_ == nullptr) return;
    clReleaseMemObject(image_);
    image_ = nullptr;
}

std::expected<ImageManager, cl_int> ImageManager::create(ContextManager& contextMgr, const Extent extent,
                                                         const ResourceType type) noexcept {
    cl_int clErr;

    auto context = contextMgr.getContext();
    const int clImageType = type == ResourceType::Read ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY;

    cl_image_desc imageDesc{};
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = extent.width();
    imageDesc.image_height = extent.height();

    cl_image_format imageFormat;
    imageFormat.image_channel_order = extent.clChannelOrder();
    imageFormat.image_channel_data_type = extent.clChannelType();

    cl_mem image = clCreateImage(context, clImageType, &imageFormat, &imageDesc, nullptr, &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    return ImageManager{std::move(image)};
}

}  // namespace clc
