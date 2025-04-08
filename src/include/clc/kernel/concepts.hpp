#pragma once

#include <concepts>

#include "clc/kernel/structs.hpp"

namespace clc {

template <typename Self>
concept CSupportGenKernelArg = requires {
    requires requires(Self& self) {
        { self.genKernelArg() } noexcept -> std::same_as<KernelArg>;
    };
};

}  // namespace vkc
