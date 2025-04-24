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

    clc::DeviceManager deviceMgr = clc::DeviceManager::create() | unwrap;
    clc::DeviceProps deviceProps = clc::DeviceProps::create(deviceMgr.getDevice()) | unwrap;
    clc::ContextManager contextMgr = clc::ContextManager::create(deviceMgr) | unwrap;
    cl_queue_properties queueProps = CL_QUEUE_PROFILING_ENABLE;
    if (deviceProps.supportOutOfOrderQueue) {
        queueProps |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
    }
    auto pQueueMgr = std::make_shared<clc::QueueManager>(
        clc::QueueManager::createWithProps(deviceMgr, contextMgr, queueProps) | unwrap);

    clc::ImageManager srcImageMgr = clc::ImageManager::createRead(contextMgr, srcImage.getExtent()) | unwrap;
    clc::ImageManager dstImageMgr = clc::ImageManager::createWrite(contextMgr, dstImage.getExtent()) | unwrap;

    std::span<std::byte> oclSource;
    if (deviceProps.hasExtension("cl_khr_fp16")) {
        oclSource = kernel::grayscaleFp16Code;
    } else {
        oclSource = kernel::grayscaleFp32Code;
    }
    clc::KernelManager kernelMgr = clc::KernelManager::create(deviceMgr, contextMgr, oclSource) | unwrap;
    std::array kernelArgs = clc::genKernelArgs(srcImageMgr, dstImageMgr);
    kernelMgr.setKernelArgs(kernelArgs) | unwrap;
    clc::CommandBufferManager commandBufferMgr = clc::CommandBufferManager::create(pQueueMgr) | unwrap;

    commandBufferMgr.uploadImageFrom(srcImageMgr, srcImage.getImageSpan(), srcImage.getExtent()) | unwrap;
    commandBufferMgr.dispatch(kernelMgr, dstImage.getExtent(), {16, 16}) | unwrap;
    commandBufferMgr.downloadImageTo(dstImageMgr, dstImage.getImageSpan(), dstImage.getExtent()) | unwrap;
    commandBufferMgr.waitTransferComplete() | unwrap;

    float elapsedTime = (float)commandBufferMgr.getDispatchElapsedTimeNs().value() / (float)1e6;
    std::println("Dispatch elapsed time: {} ms", elapsedTime);

    dstImage.saveTo("out.png") | unwrap;
}
