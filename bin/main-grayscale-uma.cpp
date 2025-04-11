#include <array>
#include <print>

#include "clc.hpp"
#include "opencl/kernel.hpp"

namespace rgs = std::ranges;

int main() {
    clc::StbImageManager srcImage{"in.png"};
    clc::StbImageManager dstImage{srcImage.getExtent()};

    clc::PlatformManager platformMgr;
    clc::DeviceManager deviceMgr{platformMgr};
    clc::ContextManager contextMgr{deviceMgr};
    auto pQueueMgr = std::make_shared<clc::QueueManager>(deviceMgr, contextMgr, CL_QUEUE_PROFILING_ENABLE);

    clc::ImageViewManager srcImageViewMgr{contextMgr, srcImage.getExtent(), clc::ResourceType::Read,
                                          srcImage.getImageSpan()};
    clc::ImageViewManager dstImageViewMgr{contextMgr, dstImage.getExtent(), clc::ResourceType::Write,
                                          dstImage.getImageSpan()};

    clc::KernelManager kernelMgr{deviceMgr, contextMgr, kernel::grayscaleOclCode};
    std::array kernelArgs = clc::genKernelArgs(srcImageViewMgr, dstImageViewMgr);
    kernelMgr.setKernelArgs(kernelArgs);
    clc::CommandBufferManager commandBufferMgr{pQueueMgr};

    commandBufferMgr.dispatch(kernelMgr, dstImage.getExtent(), {16, 16});
    auto dstSpan = commandBufferMgr.mmapForHostRead(dstImageViewMgr, dstImage.getExtent());
    commandBufferMgr.unmap(dstImageViewMgr, dstSpan);

    float elapsedTime = (float)commandBufferMgr.getDispatchElapsedTimeNs() / (float)1e6;
    std::println("Dispatch elapsed time: {} ms", elapsedTime);

    dstImage.saveTo("out.png");
}
