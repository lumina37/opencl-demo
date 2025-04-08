#include <array>

#include "clc.hpp"
#include "opencl/kernel.hpp"

int main(int argc, char** argv) {
    clc::StbImageManager srcImage{"in.png"};
    clc::StbImageManager dstImage{srcImage.getExtent()};

    clc::PlatformManager platformMgr;
    clc::DeviceManager deviceMgr{platformMgr};
    clc::ContextManager contextMgr{deviceMgr};
    auto pQueueMgr = std::make_shared<clc::QueueManager>(deviceMgr, contextMgr);

    clc::ImageManager srcImageMgr{contextMgr, srcImage.getExtent(), clc::ResourceType::Read};
    clc::ImageManager dstImageMgr{contextMgr, dstImage.getExtent(), clc::ResourceType::Write};

    clc::KernelManager kernelMgr{deviceMgr, contextMgr, kernel::grayscaleOclCode};
    std::array kernelArgs = clc::genKernelArgs(srcImageMgr, dstImageMgr);
    kernelMgr.setKernelArgs(kernelArgs);
    clc::CommandBufferManager commandBufferMgr{pQueueMgr};

    commandBufferMgr.uploadImageFrom(srcImageMgr, srcImage.getImageSpan(), srcImage.getExtent());
    commandBufferMgr.dispatch(kernelMgr, dstImage.getExtent(), {16, 16});
    commandBufferMgr.downloadImageTo(dstImageMgr, dstImage.getImageSpan(), dstImage.getExtent());
    commandBufferMgr.waitDownloadComplete();

    dstImage.saveTo("out.png");
}
