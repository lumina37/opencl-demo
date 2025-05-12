#include <array>
#include <filesystem>
#include <print>

#include "clc.hpp"
#include "opencl/kernel.hpp"
#include "clc_bin_helper.hpp"

namespace fs = std::filesystem;

int main() {
    clc::StbImageManager srcImage = clc::StbImageManager::createFromPath("in.png") | unwrap;
    clc::StbImageManager dstImage = clc::StbImageManager::createWithExtent(srcImage.getExtent()) | unwrap;

    clc::DeviceSet devices = clc::DeviceSet::create() | unwrap;
    clc::DeviceWithProps& deviceWithProps = devices.selectDefault() | unwrap;
    clc::DeviceManager& deviceMgr = deviceWithProps.getDeviceMgr();
    clc::ContextManager contextMgr = clc::ContextManager::create(deviceMgr) | unwrap;
    cl_queue_properties queueProps = CL_QUEUE_PROFILING_ENABLE;
    if (deviceWithProps.getProps().supportOutOfOrderQueue) {
        queueProps |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
    }
    clc::QueueManager queueMgr = clc::QueueManager::createWithProps(deviceMgr, contextMgr, queueProps) | unwrap;

    clc::ImageManager srcImageMgr =
        clc::ImageManager::createReadUMA(contextMgr, srcImage.getExtent(), srcImage.getImageSpan()) | unwrap;
    clc::ImageManager dstImageMgr = clc::ImageManager::createWriteUMA(contextMgr, dstImage.getExtent()) | unwrap;

    clc::KernelManager kernelMgr =
        clc::KernelManager::create(deviceMgr, contextMgr, kernel::grayscaleFp32Code) | unwrap;
    const std::array kernelArgs = clc::genKernelArgs(srcImageMgr, dstImageMgr);
    kernelMgr.setKernelArgs(kernelArgs) | unwrap;

    clc::EventManager dispatchEv = queueMgr.dispatch(kernelMgr, dstImage.getExtent(), {16, 16}, {}) | unwrap;
    std::array dispatchEvs{std::cref(dispatchEv)};
    auto dstSpan = queueMgr.mmapForHostRead(dstImageMgr, dstImage.getExtent(), dispatchEvs) | unwrap;
    std::copy(dstSpan.begin(), dstSpan.end(), dstImage.getImageSpan().begin());
    queueMgr.unmap(dstImageMgr, dstSpan) | unwrap;

    float elapsedTime = (float)dispatchEv.getElapsedTimeNs().value() / (float)1e6;
    std::println("Dispatch elapsed time: {} ms", elapsedTime);

    dstImage.saveTo("out.png") | unwrap;
}
