#pragma once

#include <cstddef>

namespace clc {

struct KernelArg {
    size_t size;
    void* ptr;
};

}
