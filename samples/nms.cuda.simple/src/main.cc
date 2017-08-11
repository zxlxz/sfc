
#include <nms/io.h>
#include "cuda/axpy.h"

int main() {
    cuda::Array<float, 1> x({ 100 });
    cuda::Array<float, 1> y({ 100 });

    // x = [0, 1, 2, ... 99]
    x <<= vlins(1.0f);

    // y = [0, 1, 2, ... 99]
    y <<= vlins(1.0f);

    // cuda invoke(my_cuda_axpy)
    nms_cuda_kfunc(my_cuda_axpy)[{100}](0.01f, x, y);

    // copy device to host
    Array<float, 1>  hx(x.size());
    hx <<= x;

    // copy device to host
    nms::Array<float, 1>  hy(y.size());
    hy <<= y;

    // print array
    io::log::info("x = [{}]", hx);
    io::log::info("y = [{}]", hy);
    return 0;
}
