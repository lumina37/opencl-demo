#include <cassert>
#include <cstddef>
#include <expected>
#include <span>
#include <utility>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/resource/type.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/image_view.hpp"
#endif

namespace clc {

ImageViewManager::ImageViewManager(cl_mem&& image) noexcept : image_(image) {}

ImageViewManager::ImageViewManager(ImageViewManager&& rhs) noexcept { image_ = std::exchange(rhs.image_, nullptr); }

ImageViewManager::~ImageViewManager() noexcept {
    if (image_ == nullptr) return;
    clReleaseMemObject(image_);
    image_ = nullptr;
}

std::expected<ImageViewManager, cl_int> ImageViewManager::create(ContextManager& contextMgr, Extent extent,
                                                                 ResourceType type,
                                                                 std::span<std::byte> hostMem) noexcept {
    assert(hostMem.size() >= extent.size());

    cl_int clErr;

    auto context = contextMgr.getContext();
    const int clImageType = CL_MEM_USE_HOST_PTR | (type == ResourceType::Read ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY);

    cl_image_desc imageDesc{};
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = extent.width();
    imageDesc.image_height = extent.height();
    imageDesc.image_row_pitch = extent.rowPitch();

    cl_image_format imageFormat;
    imageFormat.image_channel_order = extent.clChannelOrder();
    imageFormat.image_channel_data_type = extent.clChannelType();

    cl_mem image = clCreateImage(context, clImageType, &imageFormat, &imageDesc, hostMem.data(), &clErr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    return ImageViewManager{std::move(image)};
}

}  // namespace clc
