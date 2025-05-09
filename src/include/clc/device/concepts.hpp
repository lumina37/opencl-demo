#pragma once

#include <concepts>
#include <expected>

#include <CL/cl.h>

#include "clc/helper/error.hpp"

namespace clc {

template <typename Self>
concept CDeviceProps = std::is_move_constructible_v<Self> && requires(cl_device_id device) {
    // Init from
    { Self::create(device) } noexcept -> std::same_as<std::expected<Self, Error>>;
} && requires(const Self& self) {
    // Evaluate the priority score
    { self.score() } noexcept -> std::same_as<std::expected<float, Error>>;
    // Const methods
    requires requires(std::string_view extName) {
        { self.hasExtension(extName) } noexcept -> std::same_as<bool>;
    };
};

}  // namespace clc
