#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/helper/exception.hpp"
#include "clc/resource/type.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/image.hpp"
#endif

namespace clc {

ImageManager::ImageManager(ContextManager& contextMgr, const Extent extent, const ResourceType type) {
    cl_int errCode;

    auto context = contextMgr.getContext();
    const int clImageType = type == ResourceType::Read ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY;

    cl_image_desc imageDesc{};
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = extent.width();
    imageDesc.image_height = extent.height();

    cl_image_format imageFormat;
    imageFormat.image_channel_order = extent.clChannelOrder();
    imageFormat.image_channel_data_type = extent.clChannelType();

    image_ = clCreateImage(context, clImageType, &imageFormat, &imageDesc, nullptr, &errCode);
    checkError(errCode);
}

ImageManager::~ImageManager() { clReleaseMemObject(image_); }

}  // namespace clc
