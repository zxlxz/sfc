#include <nms/core/memory.h>
#include <nms/test.h>
#include <nms/util/stacktrace.h>

namespace nms
{

NMS_API void* _mnew(u64 size) {
    /*
     * @see http://en.cppreference.com/w/c/memory/malloc
     * if size == 0, the behavior is implementation defined.
     */
    if (size == 0) {
        return nullptr;
    }

    const auto ptr = static_cast<void**>(::malloc(size));
    if (ptr == nullptr) {
        throw_exception(EBadAlloc{});
    }
    return ptr;
}

NMS_API void  _mdel(void* ptr) {
    /*
     * @see: http://en.cppreference.com/w/c/memory/free
     * if ptr == nullptr, ::free do nothing.
     */
    ::free(ptr);
}

NMS_API void _mzero(void* dat, u64 size) {
    ::memset(dat, 0, size);
}

NMS_API int  _mcmp(const void* dst, const void* src, u64 size) {
    return ::memcmp(dst, src, size);
}


NMS_API void  _mcpy(void* dst, const void* src, u64 size) {
    ::memcpy(dst, src, size);
}

NMS_API void  _mmov(void* dst, const void* src, u64 size) {
    ::memmove(dst, src, size);
}

NMS_API u64 msize(const void* ptr) {
#if     defined(NMS_OS_WINDOWS)
    const auto mem_size = ::_msize(const_cast<void*>(ptr));
#elif   defined(NMS_OS_APPLE)
    const auto mem_size = ::malloc_size(ptr);
#elif   defined(NMS_OS_UNIX)
    const auto mem_size = ::malloc_usable_size(const_cast<void*>(ptr));
#endif
    return mem_size;
}

#pragma region IPool
NMS_API IPool::IPool(u64 size)
        : memsize_(min(size, u64{sizeof(void*)}))
{}

NMS_API IPool::~IPool() {
    while (trunklist_ != nullptr) {
        const auto oldtrunk = trunklist_;
        const auto newtrunk = trunklist_->next;
        _mdel(oldtrunk);
        trunklist_ = newtrunk;
    }
}

NMS_API void* IPool::malloc() {
    if (freelist_ != nullptr) {
        const auto oldhead = freelist_;
        const auto newhead = freelist_->next;
        freelist_ = newhead;
        return oldhead;
    }
    if (trunklist_ == nullptr || (trunklist_->size+memsize_) > trunksize_) {
        addTrunk();
    }

    const auto trunk = reinterpret_cast<u8*>(trunklist_);
    const auto offset= trunklist_->size;
    const auto ptr   = trunk + offset;

    trunklist_->size += memsize_;
    return ptr;
}

NMS_API void IPool::free(void* ptr) {
    auto newhead    = static_cast<FreeList*>(ptr);
    auto oldhead    = freelist_;
    newhead->next   = oldhead;
    freelist_       = newhead;
}

NMS_API void IPool::addTrunk() {
    const auto trunkdata = _mnew(trunksize_);
    const auto oldtrunk  = trunklist_;

    if (trunkdata != nullptr) {
#ifdef NMS_CC_MSVC
#pragma warning(push)
#pragma warning(disable: 6011)
#endif
        const auto newtrunk = new(trunkdata)TrunkList{ oldtrunk, trunksize_ - sizeof(TrunkList) };
#ifdef NMS_CC_MSVC
#pragma warning(pop)
#endif
        trunklist_ = newtrunk;
    }
}

#pragma endregion

}


#pragma region unittest
namespace nms
{

nms_test(memory) {
    struct Block
    {
        Block()
            : x(1), y(2.0)
        {}
    
        i64 x;
        f64 y;
        char c[24];
    };
    const u32 count = 1024;
    auto ptrs = new Block*[count];

    for (auto loop = 0; loop < 2; ++loop) {
        auto t0 = clock();
        for (auto i = 0u; i < count; ++i) {
            auto p = static_cast<Block*>(::malloc(sizeof(Block)));
            ptrs[i] = p;
        }
        auto t1 = clock();

        for (auto i = 0u; i < count; ++i) {
            free(ptrs[i]);
        }
        auto t2 = clock();

        if (loop > 0) {
            io::log::info("nms.memory: malloc {}, free {}", t1 - t0, t2 - t1);
        }
    }

    for (auto loop = 0; loop < 2; ++loop) {
        auto t0 = clock();
        for (auto i = 0u; i < count; ++i) {
            auto p = new Block();
            ptrs[i] = p;
        }
        auto t1 = clock();
        for (u32 i = 0; i < count; ++i) {
            delete ptrs[i];
        }
        auto t2 = clock();
        if (loop > 0) {
            io::log::info("nms.memory: new    {}, del  {}", t1 - t0, t2 - t1);
        }
    }

    for (auto loop = 0; loop < 2; ++loop) {
        auto t0 = clock();
        for (auto i = 0u; i < count; ++i) {
            auto p = mnew<Block>(1);
            new(p)Block();
            ptrs[i] = p;
        }
        auto t1 = clock();
        for (u32 i = 0; i < count; ++i) {
            mdel(ptrs[i]);
        }
        auto t2 = clock();
        if (loop > 0) {
            io::log::info("nms.memory: mnew   {}, mdel {}", t1 - t0, t2 - t1);
        }
    }

    delete[] ptrs;
}
}
#pragma endregion
