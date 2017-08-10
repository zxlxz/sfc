#pragma once

#include <nms/cuda.h>

using namespace nms;

struct vadd;
__kernel__ void vadd(float* y, float *a, float *b, u32 n) {
    auto id = blockIdx.x * blockDim.x + threadIdx.x;
    if (id >= n) {
        return;
    }
    y[id] = a[id] + b[id];
}
