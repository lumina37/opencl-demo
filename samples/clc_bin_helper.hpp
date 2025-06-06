#pragma once

#include <chrono>
#include <expected>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>
#include <type_traits>
#include <utility>

#include "clc.hpp"

namespace fs = std::filesystem;

class Unwrap {
public:
    template <typename T>
    friend auto operator|(std::expected<T, clc::Error>&& src, [[maybe_unused]] const Unwrap& _) {
        if (!src.has_value()) {
            const auto& err = src.error();
            const fs::path filePath{err.source.file_name()};
            const std::string fileName = filePath.filename().string();
            std::println(std::cerr, "{}:{} msg={} code={}", fileName, err.source.line(), err.msg, err.code);
            std::exit(err.code);
        }
        if constexpr (!std::is_void_v<T>) {
            return std::forward_like<T>(src.value());
        }
    }
};

constexpr auto unwrap = Unwrap();

class Timer {
public:
    void begin() noexcept { begin_ = std::chrono::steady_clock::now(); }
    void end() noexcept {
        const auto end = std::chrono::steady_clock::now();
        durationNs_ = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin_).count();
    }
    [[nodiscard]] float durationMs() const noexcept { return (float)durationNs_ / 1000000.0f; }

private:
    std::chrono::time_point<std::chrono::steady_clock> begin_;
    int64_t durationNs_{};
};
