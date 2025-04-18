#include <array>
#include <expected>
#include <format>
#include <print>

#include "clc.hpp"
#include "opencl/kernel.hpp"

class Unwrap {
public:
    template <typename E>
    static friend void operator|(std::expected<void, E>&& src, const Unwrap& _) {
        if (!src.has_value()) {
            const auto errStr = std::format("Errno: {}", src.error());
            throw std::runtime_error(errStr);
        }
    }

    template <typename T, typename E>
    static friend auto operator|(std::expected<T, E>&& src, const Unwrap& _) {
        if (!src.has_value()) {
            const auto errStr = std::format("Errno: {}", src.error());
            throw std::runtime_error(errStr);
        }
        return std::forward_like<T>(src.value());
    }
};

constexpr auto unwrap = Unwrap();

int main() {
    clc::StbImageManager srcImage = clc::StbImageManager::createFromPath("in.png") | unwrap;
    clc::StbImageManager dstImage = clc::StbImageManager::createWithExtent(srcImage.getExtent()) | unwrap;

    clc::PlatformManager platformMgr = clc::PlatformManager::create() | unwrap;
    clc::DeviceManager deviceMgr = clc::DeviceManager::create(platformMgr) | unwrap;
    clc::ContextManager contextMgr = clc::ContextManager::create(deviceMgr) | unwrap;
    auto pQueueMgr = std::make_shared<clc::QueueManager>(
        clc::QueueManager::createWithProps(deviceMgr, contextMgr, CL_QUEUE_PROFILING_ENABLE) | unwrap);

    clc::ImageViewManager srcImageViewMgr =
        clc::ImageViewManager::create(contextMgr, srcImage.getExtent(), clc::ResourceType::Read,
                                      srcImage.getImageSpan()) |
        unwrap;
    clc::ImageViewManager dstImageViewMgr =
        clc::ImageViewManager::create(contextMgr, dstImage.getExtent(), clc::ResourceType::Write,
                                      dstImage.getImageSpan()) |
        unwrap;

    clc::KernelManager kernelMgr = clc::KernelManager::create(deviceMgr, contextMgr, kernel::grayscaleOclCode) | unwrap;
    std::array kernelArgs = clc::genKernelArgs(srcImageViewMgr, dstImageViewMgr);
    kernelMgr.setKernelArgs(kernelArgs) | unwrap;
    clc::CommandBufferManager commandBufferMgr = clc::CommandBufferManager::create(pQueueMgr) | unwrap;

    commandBufferMgr.dispatch(kernelMgr, dstImage.getExtent(), {16, 16}) | unwrap;
    auto dstSpan = commandBufferMgr.mmapForHostRead(dstImageViewMgr, dstImage.getExtent()) | unwrap;
    commandBufferMgr.unmap(dstImageViewMgr, dstSpan) | unwrap;

    float elapsedTime = (float)commandBufferMgr.getDispatchElapsedTimeNs().value() / (float)1e6;
    std::println("Dispatch elapsed time: {} ms", elapsedTime);

    dstImage.saveTo("out.png") | unwrap;
}
