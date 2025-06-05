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

    clc::ImageBox srcImageBox = clc::ImageBox::createRead(contextBox, srcImage.getExtent()) | unwrap;
    clc::ImageBox dstImageBox = clc::ImageBox::createWrite(contextBox, dstImage.getExtent()) | unwrap;

    std::span<std::byte> oclSource;
    if (deviceWithProps.getProps().hasExtension("cl_khr_fp16")) {
        std::println("use fp16 kernel");
        oclSource = kernel::grayscale::fp16::code;
    } else {
        std::println("use fp32 kernel");
        oclSource = kernel::grayscale::fp32::code;
    }
    clc::KernelBox kernelBox = clc::KernelBox::create(deviceBox, contextBox, oclSource) | unwrap;
    const std::array kernelArgs = clc::genKernelArgs(srcImageBox, dstImageBox);
    kernelBox.setKernelArgs(kernelArgs) | unwrap;

    clc::EventBox uploadEv =
        queueBox.uploadImageFrom(srcImageBox, srcImage.getImageSpan(), srcImage.getExtent(), {}) | unwrap;
    std::array uploadEvs{std::cref(uploadEv)};
    clc::EventBox dispatchEv = queueBox.dispatch(kernelBox, dstImage.getExtent(), {16, 16}, uploadEvs) | unwrap;
    std::array dispatchEvs{std::cref(dispatchEv)};
    clc::EventBox downloadEv =
        queueBox.downloadImageTo(dstImageBox, dstImage.getImageSpan(), dstImage.getExtent(), dispatchEvs) | unwrap;
    std::array downloadEvs{std::cref(downloadEv)};
    clc::EventBox::wait(downloadEvs) | unwrap;

    overallTimer.end();

    float uploadTime = (float)uploadEv.getElapsedTimeNs().value() / (float)1e6;
    std::println("Upload elapsed time: {} ms", uploadTime);
    float dispatchTime = (float)dispatchEv.getElapsedTimeNs().value() / (float)1e6;
    std::println("Dispatch elapsed time: {} ms", dispatchTime);
    float downloadTime = (float)downloadEv.getElapsedTimeNs().value() / (float)1e6;
    std::println("Download elapsed time: {} ms", downloadTime);
    float overallTime = overallTimer.durationMs();
    std::println("Overall elapsed time: {} ms", overallTime);

    dstImage.saveTo("out.png") | unwrap;
}
