#include <algorithm>
#include <cstddef>
#include <memory>
#include <print>
#include <ranges>
#include <utility>
#include <vector>

#include <CL/cl.h>

#include "clc/common/defines.hpp"
#include "clc/helper/score.hpp"

#ifndef _CLC_LIB_HEADER_ONLY
#    include "clc/device/platform.hpp"
#endif

namespace clc {

namespace rgs = std::ranges;

PlatformManager::PlatformManager(cl_platform_id&& platform) noexcept : platform_(platform) {}

std::expected<PlatformManager, cl_int> PlatformManager::create() noexcept {
    cl_int clErr;

    cl_uint platformCount;
    clErr = clGetPlatformIDs(0, nullptr, &platformCount);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    auto pPlatforms = std::make_unique_for_overwrite<cl_platform_id[]>(platformCount);
    clErr = clGetPlatformIDs(platformCount, pPlatforms.get(), nullptr);
    if (clErr != CL_SUCCESS) return std::unexpected{clErr};

    const auto getPlatformScore = [](cl_platform_id platform) -> std::expected<int64_t, cl_int> {
        cl_int clErr;

        size_t extStrSize;
        clErr = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, 0, nullptr, &extStrSize);
        if (clErr != CL_SUCCESS) return std::unexpected{clErr};

        auto pExtStr = std::make_unique_for_overwrite<char[]>(extStrSize);
        clErr = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, extStrSize, pExtStr.get(), nullptr);
        if (clErr != CL_SUCCESS) return std::unexpected{clErr};

        const int64_t score = std::count(pExtStr.get(), pExtStr.get() + extStrSize, ' ');

        if constexpr (ENABLE_DEBUG) {
            size_t nameSize;
            clErr = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, nullptr, &nameSize);
            if (clErr != CL_SUCCESS) return std::unexpected{clErr};

            auto pName = std::make_unique_for_overwrite<char[]>(nameSize);
            clErr = clGetPlatformInfo(platform, CL_PLATFORM_NAME, nameSize, pName.get(), nullptr);
            if (clErr != CL_SUCCESS) return std::unexpected{clErr};

            size_t vendorSize;
            clErr = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 0, nullptr, &vendorSize);
            if (clErr != CL_SUCCESS) return std::unexpected{clErr};

            auto pVendor = std::make_unique_for_overwrite<char[]>(vendorSize);
            clErr = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, vendorSize, pVendor.get(), nullptr);
            if (clErr != CL_SUCCESS) return std::unexpected{clErr};

            size_t versionSize;
            clErr = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, nullptr, &versionSize);
            if (clErr != CL_SUCCESS) return std::unexpected{clErr};

            auto pVersion = std::make_unique_for_overwrite<char[]>(versionSize);
            clErr = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, versionSize, pVersion.get(), nullptr);
            if (clErr != CL_SUCCESS) return std::unexpected{clErr};

            std::println("Candidate platform: name={}, vendor={}, version={}, score={}", pName.get(), pVendor.get(),
                         pVersion.get(), score);
        }

        return score;
    };

    std::vector<Score<int>> scores;
    scores.reserve(platformCount);
    for (const int idx : rgs::views::iota(0, (int)platformCount)) {
        cl_platform_id platform = pPlatforms[idx];
        auto scoreRes = getPlatformScore(platform);
        if (!scoreRes) return std::unexpected{scoreRes.error()};
        scores.emplace_back(scoreRes.value(), idx);
    }

    if (scores.empty()) {
        return std::unexpected{-1};
    }

    const auto maxScoreIt = std::max_element(scores.begin(), scores.end());
    const size_t platformIdx = maxScoreIt->attachment;

    cl_platform_id platform = pPlatforms[platformIdx];
    return PlatformManager{std::move(platform)};
}

}  // namespace clc
