#include <array>
#include <print>

#include "clc.hpp"
#include "opencl/kernel.hpp"

int main() {
    clc::StbImageManager srcImage{"in.png"};
    clc::StbImageManager dstImage{srcImage.getExtent()};

    clc::PlatformManager platformMgr;
    clc::DeviceManager deviceMgr{platformMgr};
    clc::ContextManager contextMgr{deviceMgr};
    auto pQueueMgr = std::make_shared<clc::QueueManager>(deviceMgr, contextMgr, CL_QUEUE_PROFILING_ENABLE);

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

    float elapsedTime = (float)commandBufferMgr.getDispatchElapsedTimeNs() / (float)1e6;
    std::println("Dispatch elapsed time: {} ms", elapsedTime);

    dstImage.saveTo("out.png");
}
