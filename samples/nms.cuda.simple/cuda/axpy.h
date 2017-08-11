#pragma once

#ifndef _NMS_CUDA_KERNEL_H_
#include <nms/cuda.h>
#endif

using namespace nms;

__kernel__ void my_cuda_axpy(float a, View<float, 1> x, View<float, 1> y) {
    u32 id = blockIdx.x * blockDim.x + threadIdx.x;
    if (id >= x.size(0)) {
        return;
    }

    y(id) += a*x(id);
}
