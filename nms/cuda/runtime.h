#pragma once

#include <nms/cuda/base.h>

namespace nms::cuda
{

using arr_t = CUarray_st*;

/**
 * cuda runtime exception
 */
class Exception: public IException
{
  public:
    explicit Exception(u32 id)
        :id_(id)
    {}

    void format(IString& buf) const override;

  protected:
    u32 id_;
};

/**
 * cuda device
 */
class Device
{
public:
    explicit Device(i32 id)
        : id_(id) {
        const auto cnt = i32(count());

        if (id_ > cnt) {
            id_ = -1;
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
    i32 id_ = -1;

private:
    using ctx_t =   ::CUctx_st;
    static i32      gid_;
    static ctx_t*   ctx_[32];
};

/**
 * cuda stream
 */
class Stream
{
public:
    Stream() {
        _new();
    }

    ~Stream() {
        _del();
    }

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    /**
     * synchronize this stream
     */
    NMS_API void sync() const;

    /**
     * get id
     */
    auto id() const {
        return id_;
    }

    /**
     * the cuda global stream.
     */
    static Stream& global() {
        static Stream value(static_cast<CUstream_st*>(nullptr));
        return value;
    }

protected:
    CUstream_st*    id_ = nullptr;;

    explicit Stream(CUstream_st* id) {
        id_ = id;
    }

    NMS_API void _new();
    NMS_API void _del();
};

class Module
{
public:
    using   sym_t = struct CUsymbol_st*;
    using   fun_t = CUfunc_st*;

    NMS_API ~Module();
    NMS_API explicit Module(StrView ptx_src);

    /* invoke kernel */
    template<class ...Targ>
    void invoke(fun_t kernel, u32 rank, const u32 dims[], const Targ& ...args) const {
        const void* argv[] = { &args... };
        run_kernel(kernel, rank, dims, argv);
    }

    NMS_API sym_t   get_symbol(StrView name) const;
    NMS_API void    set_symbol(sym_t   symbol, const void* value, u32 size) const;

    NMS_API fun_t   get_kernel(StrView name) const;
    NMS_API void    run_kernel(fun_t   func, u32 rank, const u32 dims[], const void* argv[], Stream& stream=Stream::global()) const;

protected:
    CUmod_st* module_ = nullptr;
};

NMS_API void*_mnew(u64   size);
NMS_API void _mdel(void* data);
NMS_API void*_hnew(u64   size);
NMS_API void _hdel(void* data);

NMS_API arr_t arr_new(char type, u32 size, u32 channels, u32 rank, const u32 dims[]);
NMS_API void  arr_del(arr_t arr);

NMS_API void _mcpy  (void* dst, const void* src, u64 size, Stream& stream);
NMS_API void _h2dcpy(void* dst, const void* src, u64 size, Stream& stream);
NMS_API void _d2hcpy(void* dst, const void* src, u64 size, Stream& stream);

/*!
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
    return _mcpy(dst, src, n * sizeof(T), stream);
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
