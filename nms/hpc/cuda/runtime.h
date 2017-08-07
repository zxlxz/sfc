#pragma once

#include <nms/core.h>

#ifndef __cuda_cuda_h__
struct CUstream_st;
struct CUarray_st;
struct CUmod_st;
struct CUfunc_st;
struct CUsymbol_st;
enum   cudaError_enum {};
#endif

namespace nms::hpc::cuda
{

using arr_t = CUarray_st*;

/**
 * cuda runtime exception
 */
class Exception: public IException
{
  public:
    Exception(u32 id):id_(id)
    {}

    void format(String& buf) const override;

  protected:
    u32     id_;
};

/**
 * cuda device
 */
class Device : public INocopyable
{
  public:
    Device(u32 id):id_(id) {
        if (id_ > count()) {
            id_ = 0;
        }
    }

    /**
     * synchronize this device
     */
    NMS_API void sync() const;

    /**
     * get gpu device count
     */
    NMS_API static u32 count();

  protected:
    u32     id_     = 0;
};

/**
 * cuda stream
 */
class Stream : public INocopyable
{
  public:
    NMS_API Stream();
    NMS_API ~Stream();

    /**
     * synchronize this stream
     */
    NMS_API void sync() const;

    auto id() const {
        return id_;
    }

    /**
     * the cuda global stream.
     */
    static Stream& global() {
        static Stream value(nullptr);
        return value;
    }

  protected:
    CUstream_st*    id_;

    NMS_API explicit Stream(nullptr_t);
};

class Module : public INocopyable
{
public:
    using   sym_t = struct CUsymbol_st*;
    using   fun_t = CUfunc_st*;

    NMS_API explicit Module(StrView ptx_src);
    NMS_API ~Module();

    /* invoke lambda */
    template<class Tfunc, class Tret, class ...Targs>
    void invoke(Tfunc func, Tret& ret, const Targs& ...args) {
        static const auto kernel = get_kernel(func);
        invoke(kernel, ret, args...);
    }

    /* invoke kernel */
    template<class Tret, class ...Targs>
    void invoke(fun_t kernel, Tret& ret, const Targs& ...args) {
        static const auto rank = Tret::rank();
        const auto  dims = ret.size().data();
        const void* argv[] = { &ret, &args... };
        run_kernel(kernel, argv, rank, dims);
    }

    NMS_API sym_t   get_symbol(StrView name) const;
    NMS_API void    set_symbol(sym_t   symbol, const void* value, u32 size) const;

    NMS_API fun_t   get_kernel(StrView name) const;
    NMS_API fun_t   get_kernel(u32     index)const;
    NMS_API void    run_kernel(fun_t   func, const void* argv[], u32 rank, const u32 dims[], Stream& stream=Stream::global()) const;

protected:
    CUmod_st* module_ = nullptr;
};

NMS_API Module&  gModule();

/*!
* invoke cuda device function
* @param func:     cuda device function
*/
static auto cufun(StrView name) {
    return gModule().get_kernel(name);
}

/*!
 * invoke cuda device function
 * @param func:     cuda device function
 */
template<class ...Targ>
void invoke(Module::fun_t func, const Targ& ...arg) {
    gModule().invoke(func, lambda_cast(arg)...);
}


NMS_API void*_mnew(u64   size);
NMS_API void _mdel(void* data);
NMS_API void*_hnew(u64   size);
NMS_API void _hdel(void* data);

NMS_API arr_t arr_new(char type, u32 size, u32 channels, u32 rank, const u32 dims[]);
NMS_API void  arr_del(arr_t arr);

NMS_API void _mcpy  (void* dst, const void* src, u64 size, Stream& stream);
NMS_API void _h2dcpy(void* dst, const void* src, u64 size, Stream& stream);
NMS_API void _d2hcpy(void* dst, const void* src, u64 size, Stream& stream);

/**
 * alloc GPU device memory
 *
 * @param size: request allocation size in bytes.
 *
 * @return device pointer
 */
template<class T>T*  mnew(u64 size) {
    return static_cast<T*>(_mnew(sizeof(T)*size));
}

/**
 * free GPU device memory
 *
 * @param dptr: device pointer
 */
template<class T>
void mdel(T*  dptr) {
    _mdel(dptr);
}

/**
 * alloc GPU device memory
 *
 * @param size: request allocation size in bytes.
 *
 * @return device pointer
 */
template<class T>
T* hnew(u64 size) {
    return static_cast<T*>(_hnew(sizeof(T)*size));
}


/**
 * alloc host page lock memory
 *
 * @return device pointer
 */
template<class T>
void hdel(T*  data) {
    _hdel(data);
}

/**
 * alloc cuda array
 * @param dims:  array dims
 * 
 * @return the cuda array
 */
template<class T, u32 N>
arr_t arr_new(u32(&dims)[N]) {
    const auto type = $is<$sint, T>() ? 'i' : $is<$uint, T>() ? 'u' : $is<$float, T>() ? 'f' : '?';
    const auto size = u32(sizeof(T));
    const auto rank = N;
    const auto arr = arr_new(type, size, rank, dims);
    return arr;
}

/**
 * memory copy using uva
 *
 * @param dst:      dst
 * @param src:      src
 * @param n:        number of elements
 * @param stream:   cuda stream, default is global
 *
 * @sa Stream::global
 */
template<class T>
void mcpy(T*  dst, const T* src, u64 n, Stream& stream = Stream::global()) {
    return _mcpy  (dst, src, n * sizeof(T), stream);
}

/**
 * memory copy from host to device
 *
 * @param dst:      dst
 * @param src:      src
 * @param n:        number of elements
 * @param stream:   cuda stream, default is global
 *
 * @sa Stream::global
 */
template<class T>
void h2dcpy(T*  dst, const T* src, u64 n, Stream& stream = Stream::global()) {
    return _h2dcpy(dst, src, n * sizeof(T), stream);
}

/**
 * memory copy from device to host
 * @param dst:      dst
 * @param src:      src
 * @param n:        number of elements
 * @param stream:   cuda stream, default is global
 *
 * @sa Stream::global
 */
template<class T>
void d2hcpy(T*  dst, const T* src, u64 n, Stream& stream = Stream::global() ) {
    return _d2hcpy(dst, src, n * sizeof(T), stream);
}


enum TexAddressMode
{
    Wrap = 0,
    Clamp = 1,
    Mirror = 2,
    Border = 3,
};

enum TexFilterMode
{
    Point = 0,
    Liner = 1,
};

NMS_API u64   tex_new(arr_t arr, TexAddressMode border_mode, TexFilterMode filter_mode);
NMS_API void  tex_del(u64 obj);


}
