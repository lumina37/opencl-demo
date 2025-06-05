#include <array>
#include <filesystem>
#include <print>

#include "clc.hpp"
#include "clc_bin_helper.hpp"
#include "kernel.hpp"

namespace fs = std::filesystem;

int main() {
    clc::StbImageBox srcImage = clc::StbImageBox::createFromPath("in.png") | unwrap;
    clc::StbImageBox dstImage = clc::StbImageBox::createWithExtent(srcImage.getExtent()) | unwrap;

    clc::DeviceSet devices = clc::DeviceSet::create() | unwrap;
    clc::DeviceWithProps& deviceWithProps = devices.selectDefault() | unwrap;
    clc::DeviceBox& deviceBox = deviceWithProps.getDeviceBox();
    clc::ContextBox contextBox = clc::ContextBox::create(deviceBox) | unwrap;
    cl_queue_properties queueProps = CL_QUEUE_PROFILING_ENABLE;
    if (deviceWithProps.getProps().supportOutOfOrderQueue) {
        queueProps |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
    }
    clc::QueueBox queueBox = clc::QueueBox::createWithProps(deviceBox, contextBox, queueProps) | unwrap;

    Timer overallTimer;
    overallTimer.begin();

    clc::ImageBox srcImageBox =
        clc::ImageBox::createReadUMA(contextBox, srcImage.getExtent(), srcImage.getImageSpan()) | unwrap;
    clc::ImageBox dstImageBox = clc::ImageBox::createWriteUMA(contextBox, dstImage.getExtent()) | unwrap;

    clc::KernelBox kernelBox = clc::KernelBox::create(deviceBox, contextBox, kernel::grayscale::fp32::code) | unwrap;
    const std::array kernelArgs = clc::genKernelArgs(srcImageBox, dstImageBox);
    kernelBox.setKernelArgs(kernelArgs) | unwrap;

    clc::EventBox dispatchEv = queueBox.dispatch(kernelBox, dstImage.getExtent(), {16, 16}, {}) | unwrap;
    std::array dispatchEvs{std::cref(dispatchEv)};
    auto dstSpan = queueBox.mmapForHostRead(dstImageBox, dstImage.getExtent(), dispatchEvs) | unwrap;
    std::copy(dstSpan.begin(), dstSpan.end(), dstImage.getImageSpan().begin());
    queueBox.unmap(dstImageBox, dstSpan) | unwrap;

    overallTimer.end();

    float elapsedTime = (float)dispatchEv.getElapsedTimeNs().value() / (float)1e6;
    std::println("Dispatch elapsed time: {} ms", elapsedTime);
    float overallTime = overallTimer.durationMs();
    std::println("Overall elapsed time: {} ms", overallTime);

    dstImage.saveTo("out.png") | unwrap;
}
