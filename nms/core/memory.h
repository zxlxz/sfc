#pragma once

#include <nms/core/type.h>
#include <nms/core/trait.h>

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void* operator new(size_t size, void* address)   throw() {    (void)size;    return address; }
inline void  operator delete(void*, void*)              throw() {}
#endif

#ifndef __PLACEMENT_VEC_NEW_INLINE
#define __PLACEMENT_VEC_NEW_INLINE
inline void* operator new[](size_t size, void* address) throw() {   (void)size;     return address; }
inline void operator delete[](void*, void*)             throw() {}
#endif

#ifdef NMS_OS_MACOS
extern "C"
{
size_t malloc_size(const void* ptr);
}
#endif

namespace nms
{

NMS_API void* _mnew  (u64 size);
NMS_API void  _mdel  (void* dat);
NMS_API void  _mzero (void* dat, u64 size);
NMS_API void  _mcpy  (void* dst, const void* src, u64 size);
NMS_API void  _mmov  (void* dst, const void* src, u64 size);

class EBadAlloc: public IException
{};

/* allocation */
template<class T>
T* mnew(u64 n){
    const auto size = n * sizeof(T);
    const auto ptr  = static_cast<T*>(_mnew(size));

    if (ptr == nullptr) {
        throw EBadAlloc{};
    }
    return ptr;
}

/* deallocation */
template<class T>
__forceinline void mdel(T* dat) {
    return _mdel(dat);
}

/* fills a buffer with 0 */
template<class T>
__forceinline T*  mzero(T* dat, u64 n) {
    _mzero(dat, n * sizeof(T));
    return dat;
}

/* copies one buffer to another */
template<class T>
__forceinline void mcpy(T* dst, const T* src, u64 n) {
    if ($is_pod<T>) {
        _mcpy(dst, src, n * sizeof(T));
    }
    else {
        for (u64 i = 0; i < n; ++i) {
            new(&dst[i])T(src[i]);
        }
    }
}

/* moves one buffer to another */
template<class T>
__forceinline void mmov (T* dst, T* src, u64 n) {
    if ($is_pod<T>) {
        _mmov(dst, src, n * sizeof(T));
    }
    else {
        for (u64 i = 0u; i < n; ++i) {
            new(&dst[i])T(static_cast<T&&>(src[i]));
        }
    }
}

__forceinline u64 msize(const void* ptr) {
#ifdef NMS_OS_WINDOWS
    const auto mem_size = reinterpret_cast<const u64*>(ptr)[-2];
#endif
#ifdef NMS_OS_MACOS
    const auto mem_size = ::malloc_size(ptr);
#endif
    return mem_size;
}

class IPool
    : public INocopyable
{
    struct FreeList {
        FreeList* next = nullptr;
    };

    struct TrunkList {
        TrunkList*  next;
        u64         size;
    };

public:
    IPool() noexcept
    {}

    IPool(IPool& rhs) noexcept
        : freelist_(rhs.freelist_)
        , trunklist_(rhs.trunklist_)
        , trunksize_(rhs.trunksize_)
        , memsize_(rhs.memsize_)
    {
        rhs.freelist_ = nullptr;
        rhs.trunklist_ = nullptr;
    }

    NMS_API IPool(u64 size);
    NMS_API ~IPool();

    NMS_API void* malloc();
    NMS_API void free(void* ptr);

protected:
    FreeList*   freelist_   = nullptr;
    TrunkList*  trunklist_  = nullptr;
    u64         trunksize_  = 128 * 1024 * 1024;
    u64         memsize_    = 0;

    NMS_API void addTrunk();
};

template<class T>
class Pool
    : protected IPool
{
public:
    using base = IPool;

    Pool(): base(sizeof(T))
    {}

    ~Pool()
    {}

    Pool(Pool&& rhs) noexcept
        : base(move(rhs))
    {}

    T* malloc() {
        const auto ptr = base::malloc();
        return reinterpret_cast<T*>(ptr);
    }

    void free(T* ptr) {
        base::free(ptr);
    }
};

template<class T, u32 N>
class Pool<T[N]>
    : protected IPool
{
public:
    using base = IPool;

    Pool(): base(sizeof(T)*N)
    {}

    ~Pool()
    {}

    Pool(Pool&& rhs) noexcept
        : base(move(rhs))
    {}

    T* malloc() {
        const auto ptr = base::malloc();
        return reinterpret_cast<T*>(ptr);
    }

    void free(T* ptr) {
        base::free(ptr);
    }
};

}
