#pragma once

#include <nms/core.h>
#include <nms/math.h>

#ifndef __cuda_cuda_h__
struct CUstream_st;
struct CUarray_st;
struct CUmod_st;
struct CUfunc_st;
struct CUsymbol_st;
struct CUctx_st;

enum   cudaError_enum {};
#endif

#ifndef __NMS_CUDA_KERNEL_H_
#define __kernel__ __forceinline
#endif

#ifndef __CUDACC__
struct dim3
{
    unsigned x;
    unsigned y;
    unsigned z;
};

extern dim3 gridIdx;
extern dim3 blockIdx;
extern dim3 threadIdx;

extern dim3 gridDim;
extern dim3 blockDim;

#endif

namespace nms::cuda
{

class Program;
class Module;

namespace device
{
template<class T, u32 N = 1>
class Array;
}
using device::Array;

namespace host
{
template<class T, u32 N = 1>
class Array;
}


}