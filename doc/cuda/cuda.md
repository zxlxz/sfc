
# cuda is simple

有两种方式使用cuda
- 数学表达式
- 直接调用cuda kernel函数。

## 1. 数学表达式
凡是
## cuda-kernel.cu
```cpp
__global__ void cuda_kernel(View<float,2> a) {
    auto x = blockIdx.x;
    auto y = threadIdx.x;
    a(x, y) = x+0.1 * y;                        // using View<float,2> as matlab.
}
```

## main.cpp
```cpp

using namespace nms::hpc;
using namespace nms::hpc::cuda;

auto arr_2d = cuda::Array<float,2>({32, 32});   // create an 32x32 float array.
arr_2d <<= 1.0f;                                // all elements in arr_2d now is 1.0f.

arr_2d <<= vlins(1.0f, 0.1f);                   // for each: arr_2d(x, y) = x+0.1*y;
arr_2d <<= vsin(arr_2d);                        // for each: arr_2d(x, y) = sin(arr2d(x, y));

cuda_run_kernel<<<32, 32>>>(arr2d);             // invoke cuda.

```
