#include <array>
#include <filesystem>
#include <iostream>
#include <print>

#include "clc.hpp"
#include "opencl/kernel.hpp"

namespace fs = std::filesystem;

class Unwrap {
public:
    template <typename T>
    static friend auto operator|(std::expected<T, clc::Error>&& src, const Unwrap& _) {
        if (!src.has_value()) {
            const auto& err = src.error();
            const fs::path filePath{err.source.file_name()};
            const std::string fileName = filePath.filename().string();
            std::println(std::cerr, "{}:{} msg={} clErr={}", fileName, err.source.line(), err.msg, err.code);
            std::exit(err.code);
        }
        if constexpr (!std::is_void_v<T>) {
            return std::forward_like<T>(src.value());
        }
    }
};

constexpr auto unwrap = Unwrap();

int main() {
    clc::StbImageManager srcImage = clc::StbImageManager::createFromPath("in.png") | unwrap;
    clc::StbImageManager dstImage = clc::StbImageManager::createWithExtent(srcImage.getExtent()) | unwrap;

    clc::Devices devices = clc::Devices::create() | unwrap;
    clc::DeviceWithProps& deviceWithProps = (devices.pickDefault() | unwrap).get();
    clc::DeviceManager& deviceMgr = deviceWithProps.getManager();
    clc::ContextManager contextMgr = clc::ContextManager::create(deviceMgr) | unwrap;
    cl_queue_properties queueProps = CL_QUEUE_PROFILING_ENABLE;
    if (deviceWithProps.getProps().supportOutOfOrderQueue) {
        queueProps |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
    }
    clc::QueueManager queueMgr = clc::QueueManager::createWithProps(deviceMgr, contextMgr, queueProps) | unwrap;

    clc::ImageManager srcImageMgr = clc::ImageManager::createRead(contextMgr, srcImage.getExtent()) | unwrap;
    clc::ImageManager dstImageMgr = clc::ImageManager::createWrite(contextMgr, dstImage.getExtent()) | unwrap;

    std::span<std::byte> oclSource;
    if (deviceWithProps.getProps().hasExtension("cl_khr_fp16")) {
        std::println("use fp16 kernel");
        oclSource = kernel::grayscaleFp16Code;
    } else {
        std::println("use fp32 kernel");
        oclSource = kernel::grayscaleFp32Code;
    }
    clc::KernelManager kernelMgr = clc::KernelManager::create(deviceMgr, contextMgr, oclSource) | unwrap;
    std::array kernelArgs = clc::genKernelArgs(srcImageMgr, dstImageMgr);
    kernelMgr.setKernelArgs(kernelArgs) | unwrap;

    clc::EventManager uploadEv =
        queueMgr.uploadImageFrom(srcImageMgr, srcImage.getImageSpan(), srcImage.getExtent(), {}) | unwrap;
    std::array uploadEvs{std::cref(uploadEv)};
    clc::EventManager dispatchEv = queueMgr.dispatch(kernelMgr, dstImage.getExtent(), {16, 16}, uploadEvs) | unwrap;
    std::array dispatchEvs{std::cref(dispatchEv)};
    clc::EventManager downloadEv =
        queueMgr.downloadImageTo(dstImageMgr, dstImage.getImageSpan(), dstImage.getExtent(), dispatchEvs) | unwrap;
    std::array downloadEvs{std::cref(downloadEv)};
    clc::EventManager::wait(downloadEvs) | unwrap;

    float uploadTime = (float)uploadEv.getElapsedTimeNs().value() / (float)1e6;
    std::println("Upload elapsed time: {} ms", uploadTime);
    float dispatchTime = (float)dispatchEv.getElapsedTimeNs().value() / (float)1e6;
    std::println("Dispatch elapsed time: {} ms", dispatchTime);
    float downloadTime = (float)downloadEv.getElapsedTimeNs().value() / (float)1e6;
    std::println("Download elapsed time: {} ms", downloadTime);

    dstImage.saveTo("out.png") | unwrap;
}
