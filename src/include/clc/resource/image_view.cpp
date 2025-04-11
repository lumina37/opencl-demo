#include <cassert>
#include <cstddef>
#include <span>

#include <CL/cl.h>

#include "clc/device/context.hpp"
#include "clc/extent.hpp"
#include "clc/helper/exception.hpp"
#include "clc/resource/type.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/resource/image_view.hpp"
#endif

namespace clc {

ImageViewManager::ImageViewManager(ContextManager& contextMgr, const Extent extent, const ResourceType type,
                                   const std::span<std::byte> hostMem) {
    assert(hostMem.size() >= extent.size());

    cl_int errCode;

    auto context = contextMgr.getContext();
    const int clImageType = CL_MEM_USE_HOST_PTR | (type == ResourceType::Read ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY);

    cl_image_desc imageDesc{};
    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = extent.width();
    imageDesc.image_height = extent.height();
    imageDesc.image_row_pitch = extent.rowPitch();

    cl_image_format imageFormat;
    imageFormat.image_channel_order = extent.clChannelOrder();
    imageFormat.image_channel_data_type = CL_UNORM_INT8;

    image_ = clCreateImage(context, clImageType, &imageFormat, &imageDesc, hostMem.data(), &errCode);
    checkError(errCode);
}

ImageViewManager::~ImageViewManager() { clReleaseMemObject(image_); }

}  // namespace clc
