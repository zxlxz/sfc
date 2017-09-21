#pragma once

#include <nms/core/type.h>
#include <nms/core/trait.h>
#include <nms/core/exception.h>

namespace nms
{

NMS_API void* _mnew(u64 size);
NMS_API void  _mdel(void* dat);
NMS_API void  _mzero(void* dat, u64 size);
NMS_API void  _mcpy(void* dst, const void* src, u64 size);
NMS_API void  _mmov(void* dst, const void* src, u64 size);
NMS_API int   _mcmp(const void* lhs, const void* rhs, u64 size);

NMS_API void* _anew(u64 size, u64 align);
NMS_API void  _adel(void* dat);

NMS_API u64   msize(const void* ptr);

class EBadAlloc : public IException
{};

/* allocation */
template<class T>
T* mnew(u64 cnt) {
    const auto size = cnt * sizeof(T);
    const auto addr = _mnew(size);
    return static_cast<T*>(addr);
}

/* deallocation */
template<class T>
void mdel(T* ptr) {
    _mdel(ptr);
}

/* fills a buffer with 0 */
template<class T>
T*  mzero(T* ptr, u64 cnt) {
    _mzero(ptr, cnt * sizeof(T));
    return ptr;
}

/* test one buffer to another */
template<class T>
void mcmp(const T* dst, const T* src, u64 cnt) {
    _mcmp(dst, src, cnt * sizeof(T));
}

/* copies one buffer to another */
template<class T, class Isize>
void mcpy(T* dst, const T* src, Isize cnt) {
    for (Isize i = 0; i < cnt; ++i) {
        new(&dst[i])T(src[i]);
    }
}

/* moves one buffer to another */
template<class T, class Isize>
void mmov(T* dst, T* src, Isize cnt) {
    for (Isize i = 0; i < cnt; ++i) {
        new(&dst[i])T(static_cast<T&&>(src[i]));
    }
}


/* align malloc */
template<class T>
T* anew(u64 cnt, u64 align) {
    const auto size = cnt*sizeof(T);
    const auto addr = _anew(size, align);
    return static_cast<T*>(addr);
}

/* align free */
template<class T>
void adel(T* ptr) {
    _adel(ptr);
}

}
