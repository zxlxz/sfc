static const char nms_cuda_kernel_src[] = R"(

#ifndef _NMS_CUDA_KERNEL_H_
#define _NMS_CUDA_KERNEL_H_

#ifndef __constant__
#define __constant__
#endif

#ifndef __device__
#define __device__
#endif

#ifndef __global__
#define __global__
#endif

#ifndef __kernel__
#define __kernel__ extern "C" __global__
#endif

/* nms */
namespace nms {

#ifdef _MSC_VER
using   i8  = __int8;   using   u8  = unsigned __int8;
using   i16 = __int16;  using   u16 = unsigned __int16;
using   i32 = __int32;  using   u32 = unsigned __int32;
using   i64 = __int64;  using   u64 = unsigned __int64;
#else
using   i8  = signed char;  using   u8  = unsigned char;
using   i16 = short;        using   u16 = unsigned short;
using   i32 = int;          using   u32 = unsigned int;
using   i64 = long;         using   u64 = unsigned long;
#endif

using f32 = float;
using f64 = double;

#ifdef __CUDA_CC__
using u16x2 = ushort2;
using u16x3 = ushort3;
using u16x4 = ushort4;

using u32x2 = uint2;
using u32x3 = uint3;
using u32x4 = uint4;

using i16x2 = short2;
using i16x3 = short3;
using i16x4 = short4;

using i32x2 = int2;
using i32x3 = int3;
using i32x4 = int4;

using f32x2 = float2;
using f32x3 = float3;
using f32x4 = float4;

using f64x2 = double2;
using f64x3 = double3;
using f64x4 = double4;
#endif

template<class T>               constexpr T max(T t)               { return t; }
template<class T, class ...U>   constexpr T max(T a, T b, U... u)  { return max(a>b?a:b, u...); }

template<u32 N>
struct Tver {};

template<class T, u32 N>
struct View
{
    T*  data_;
    u32 size_[N];
    u32 stride_[N];

    static constexpr u32 rank()     { return N;         }
    u32 size(u32 i) const           { return size_[i];  }

    template<class X>                   T  operator()(X x)           const { return data_[x*stride_[0]];                                 }
    template<class X, class Y>          T  operator()(X x, Y y)      const { return data_[x*stride_[0] + y*stride_[1]];                  }
    template<class X, class Y, class Z> T  operator()(X x, Y y, Z z) const { return data_[x*stride_[0] + y*stride_[1] + z*stride_[2]];   }

    template<class X>                   T& operator()(X x)                 { return data_[x*stride_[0]];                                 }
    template<class X, class Y>          T& operator()(X x, Y y)            { return data_[x*stride_[0] + y*stride_[1]];                  }
    template<class X, class Y, class Z> T& operator()(X x, Y y, Z z)       { return data_[x*stride_[0] + y*stride_[1] + z*stride_[2]];   }
};

}

/* nms::math */
namespace nms{ namespace math{

template<class T>
struct Scalar
{
    T   t;

    static constexpr u32 rank()             { return 0; }
    constexpr        u32 size(u32 i) const  { return 0; }

    template<class ...I>
    T operator()(I...) const {
        return t;
    }
};

template<class Tfunc, class ...Targs>
struct Parallel;

template<class Tfunc, class A>
struct Parallel<Tfunc, A>
{
    A   a;

    static constexpr u32 rank()             { return A::rank(); }
    constexpr        u32 size(u32 i) const  { return a.size(i); }

    template<class ...I>
    auto operator()(I ...idx) const -> decltype(f(a(idx...))) {
        return Tfunc::run(a(idx...));
    }
};

template<class Tfunc, class A, class B>
struct Parallel<Tfunc, A, B>
{
    A   a;
    B   b;

    static constexpr u32 rank()             { return max(A::rank(), B::rank()); }
    constexpr        u32 size(u32 i) const  { return max(a.size(i), b.size(i)); }

    template<class ...I>
    auto operator()(I ...idx) const noexcept->decltype(f(a(idx...), b(idx...))) {
        return Tfunc::run(a(idx...), b(idx...));
    }

};

template<class Tfunc, class ...Ts>
struct Reduce;

template<class Tfunc, class T>
struct Reduce<Tfunc, T>
{
    T   t;

    static constexpr u32 rank()             { return T::rank() - 1; }
    constexpr        u32 size(u32 i) const  { return t.size(i + 1); }

    template<class ...I>
    auto operator()(I ...idx) const noexcept -> decltype(t(0, idx...)) {
        const auto  n = t.size(0);
        auto        ret = t(0, idx...);
        for (u32 i = 1; i < n; ++i) {
            ret = Tfunc::run(ret, t(i, idx...));
        }

        return ret;
    }
};

template<class T, u32 N>
struct Vline
{
    T k[N];

    static constexpr u32 rank()         { return N; }
    static constexpr u32 size(u32 i)    { return 0; }

    template<class I> T operator()(I x)           const noexcept { return T(x)*k[0]; }
    template<class I> T operator()(I x, I y)      const noexcept { return T(x)*k[0] + T(y)*k[1]; }
    template<class I> T operator()(I x, I y, I z) const noexcept { return T(x)*k[0] + T(y)*k[1] + T(z)*k[2]; }
};

template<class T>
struct Veye
{
    static constexpr u32 rank()         { return 0; }
    static constexpr u32 size(u32 i)    { return 0; }

    template<class I> T operator()(I x)           const noexcept { return T(1); }
    template<class I> T operator()(I x, I y)      const noexcept { return (x == y) ? T(1) : T(0); }
    template<class I> T operator()(I x, I y, I z) const noexcept { return (x == y && y == z) ? T(1) : T(0); }
};

}}

/* nms::math::lambda */
namespace nms { namespace math {
// [+-]a
struct Pos { template<class T> __device__ static T run(T t) noexcept { return +t; } };
struct Neg { template<class T> __device__ static T run(T t) noexcept { return -t; } };

// abs, sqrt, pow2
struct Abs  { template<class T> __device__ static T run(T t) noexcept { return abs(t);   } };
struct Sqrt { template<class T> __device__ static T run(T t) noexcept { return sqrt(t);  } };
struct RSqrt{ template<class T> __device__ static T run(T t) noexcept { return rsqrt(t); } };
struct Pow2 { template<class T> __device__ static T run(T t) noexcept { return t*t;      } };

// [sin,cos,tan](a)
struct Sin { template<class T> __device__ static T run(T t) noexcept { return sin(t); } };
struct Cos { template<class T> __device__ static T run(T t) noexcept { return cos(t); } };
struct Tan { template<class T> __device__ static T run(T t) noexcept { return tan(t); } };

// [sin,cos,tan](a)
struct ASin{ template<class T> __device__ static T run(T t) noexcept { return asin(t); } };
struct ACos{ template<class T> __device__ static T run(T t) noexcept { return acos(t); } };
struct ATan{ template<class T> __device__ static T run(T t) noexcept { return atan(t); } };

// a [+,-,*,/]b
struct Add { template<class A, class B> __device__ static auto run(A a, B b) noexcept ->decltype(a+b) { return a + b; } };
struct Sub { template<class A, class B> __device__ static auto run(A a, B b) noexcept ->decltype(a-b) { return a - b; } };
struct Mul { template<class A, class B> __device__ static auto run(A a, B b) noexcept ->decltype(a*b) { return a * b; } };
struct Div { template<class A, class B> __device__ static auto run(A a, B b) noexcept ->decltype(a/b) { return a / b; } };
struct Pow { template<class A, class B> __device__ static auto run(A a, B b) noexcept ->decltype(a*b) { return pow(a, b); } };

// a [==,!=,<, >, <=, >=] b
struct Eq  { template<class A, class B> __device__ static bool run(A a, B b) noexcept { return a == b; } };
struct Neq { template<class A, class B> __device__ static bool run(A a, B b) noexcept { return a != b; } };
struct Lt  { template<class A, class B> __device__ static bool run(A a, B b) noexcept { return a <  b; } };
struct Gt  { template<class A, class B> __device__ static bool run(A a, B b) noexcept { return a >  b; } };
struct Le  { template<class A, class B> __device__ static bool run(A a, B b) noexcept { return a <= b; } };
struct Ge  { template<class A, class B> __device__ static bool run(A a, B b) noexcept { return a >= b; } };

// sum,min,max
struct Min { template<class T> __device__ static T run(const T& a, const T& b) noexcept { return a <= b ? a : b; }   };
struct Max { template<class T> __device__ static T run(const T& a, const T& b) noexcept { return a >= b ? a : b; }   };

// *=
struct Ass2{ template<class D, class S> __device__ static D& run(D& d, S s) noexcept { return d  = s; } };
struct Add2{ template<class D, class S> __device__ static D& run(D& d, S s) noexcept { return d += s; } };
struct Sub2{ template<class D, class S> __device__ static D& run(D& d, S s) noexcept { return d -= s; } };
struct Mul2{ template<class D, class S> __device__ static D& run(D& d, S s) noexcept { return d *= s; } };
struct Div2{ template<class D, class S> __device__ static D& run(D& d, S s) noexcept { return d /= s; } };
} }

/* nms::cuda */
namespace nms{ namespace cuda{

using nms::View;

#ifndef __CUDACC__
struct dim3
{
    unsigned x;
    unsigned y;
    unsigned z;
};

dim3 blockIdx;
dim3 threadIdx;
dim3 gridDim;
dim3 blockDim;
#endif


template<class T, u32 N>
struct TexView;

template<class T>
struct TexView<T,1>
{
public:
    __device__ T operator()(f32 x) const { T val; tex1D(&val, obj_, x);              return val; }
    __device__ T operator()(u32 x) const { T val; tex1D(&val, obj_, f32(x)+0.5f);    return val; }

protected:
    u64 obj_ = 0;   // cuda texture object
};

template<class T>
struct TexView<T,2>
{
public:
    __device__ T operator()(f32 x, f32 y) const { T val; tex2D(&val, obj_, x, y);                        return val; }
    __device__ T operator()(u32 x, u32 y) const { T val; tex2D(&val, obj_, f32(x)+0.5f, f32(y)+0.5f);    return val; }

protected:
    u64 obj_ = 0;   // cuda texture object
};

template<class T>
struct TexView<T,3>
{
public:
    __device__ T operator()(f32 x, f32 y, f32 z) const { T val; tex3D(&val, obj_, x, y, z);                                return val; }
    __device__ T operator()(u32 x, u32 y, u32 z) const { T val; tex3D(&val, obj_, f32(x)+0.5f, f32(y)+0.5f, f32(z)+0.5f);  return val; }

protected:
    u64 obj_ = 0;   // cuda texture object
};

template<class Tfunc, class Tret, class Targ>
__device__ void foreach_switch(Tret& ret, const Targ& arg, Tver<1>) {
    const auto x = blockIdx.x*blockDim.x + threadIdx.x;

    if (x >= ret.size(0)) return;

    Tfunc::run(ret(x), (x));
}

template<class Tfunc, class Tret, class Targ>
__device__ void foreach_switch(Tret& ret, const Targ& arg, Tver<2>) {
    const auto x = blockIdx.x*blockDim.x + threadIdx.x;
    const auto y = blockIdx.y*blockDim.y + threadIdx.y;

    if (x >= ret.size(0) || y >= ret.size(1) ) return;

    Tfunc::run(ret(x,y), arg(x,y));
}

template<class Tfunc, class Tret, class Targ>
__device__ void foreach_switch(Tret& ret, const Targ& arg, Tver<3>) {
    const auto x = blockIdx.x*blockDim.x + threadIdx.x;
    const auto y = blockIdx.y*blockDim.y + threadIdx.y;
    const auto z = blockIdx.z*blockDim.z + threadIdx.z;

    if (x >= ret.size(0) || y >= ret.size(1) || z >= ret.size(2) ) return;

    Tfunc::run(ret(x,y,z),arg(x,y,z));
}

template<class Tfunc, class Tret, class Targ>
__device__ void foreach(Tret& ret, const Targ& arg) {
    foreach_switch<Tfunc>(ret, arg, Tver<Tret::rank()>{});
}

}}

using namespace nms;
using namespace nms::cuda;
using namespace nms::math;

#endif

)";
