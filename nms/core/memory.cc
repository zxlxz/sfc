#include <nms/test.h>

using namespace nms;

extern "C" {
#ifdef NMS_OS_WINDOWS
    enum {
        PROT_READ      = 0x04,
        PROT_WRITE     = 0x04,
    };

    enum {
        SEC_COMMIT    = 0x8000000,
        SEC_RESERVE   = 0x4000000
    };

    enum {
        MAP_SHARED = 0x1
    };

    /*!
     * Microsoft Memory Management Functions
     * https://msdn.microsoft.com/en-us/library/aa366781(v=vs.85).aspx
     */
    int   CloseHandle(void* handle);
    void* CreateFileMappingA(void* hFile, void* attributes, i32 flProtect, u32 dwMaximumSizeHigh, u32 dwMaximumSizeLow, const char* lpName);
    void* MapViewOfFile(void* hFileMappingObject, u32 dwDesiredAccess, u32 dwFileOffsetHigh, u32 dwFileOffsetLow, u64 dwNumberOfBytesToMap);
    int   UnmapViewOfFile(void* lpBaseAddress);
    void* VirtualAlloc(void* addr, size_t size, i32 type, i32 prot);
    void* VirtualFree(void* addr, size_t size, i32 type);

    static void* mmap(void* base, u64 size, int prot, int flags, int fid, u64 offset) {
        (void)base;
        (void)flags;

        // map
        const u32 size_high = (size >> 32);
        const u32 size_low = (size << 32) >> 32;

        auto hfile = reinterpret_cast<void*>(_get_osfhandle(fid));
        auto hmmap = CreateFileMappingA(hfile, nullptr, prot, size_high, size_low, nullptr);

        // view
        const u32 offset_high = (offset >> 32);
        const u32 offset_low = (offset << 32) >> 32;
        const u32 file_map_write = 0x0002;
        auto ptr = MapViewOfFile(hmmap, file_map_write, offset_high, offset_low, size);
        if (ptr == nullptr) {
            return nullptr;
        }

        return ptr;
    }

    static int munmap(void* ptr, size_t size) {
        (void)size;
        auto ret = ::UnmapViewOfFile(ptr);
        return ret == 0 ? 1 : 0;
    }

    static void muse(void *ptr, size_t size) {
        const auto mem_commit       = 0x00001000;
        const auto mem_large_pages  = 0x20000000;   // 2MB
        ::VirtualAlloc(ptr, size, mem_commit | mem_large_pages, PROT_WRITE);
    }

    static void munuse(void* ptr, size_t size) {
        const auto mem_decommit = 0x4000;
    #pragma warning(push)
    #pragma warning(disable: 6250)
        ::VirtualFree(ptr, size, mem_decommit);
    #pragma warning(pop)
    }


#else
    enum {
        SEC_RESERVE = 0x0
    };
#endif
}

namespace nms
{

static __forceinline bool _mem_debug() {
    static const auto str = ::getenv("NMS_MEM_DEBUG");
    static const auto cond = str != nullptr && str[0] == '1';
    return cond;
}

NMS_API void* _mnew(u64 size) {
    /*
     * @see http://en.cppreference.com/w/c/memory/malloc
     * if size == 0, the behavior is implementation defined.
     */
    if (size == 0) {
        return nullptr;
    }

    if (_mem_debug()) {
        auto msize = size + 16;
        auto addr = static_cast<char*>(::malloc(msize));
        if (addr == nullptr) {
            NMS_THROW(EBadAlloc{});
        }

        for (auto i = 0u; i < 8; ++i) {
            addr[i]             = '[';
            addr[msize - 1 - i] = ']';
        }
        auto ptr = addr + 8;

        printf("[++]  mem: %p %lld\n", ptr, ullong(size));
        return ptr;
    }
    else {
        auto ptr = static_cast<void*>(::malloc(size));
        if (ptr == nullptr) {
            NMS_THROW(EBadAlloc{});
        }
        return ptr;
    }
}

NMS_API void  _mdel(void* ptr) {
    /*
     * @see: http://en.cppreference.com/w/c/memory/free
     * if ptr == nullptr, ::free do nothing.
     */
    if (_mem_debug()) {
        auto size = msize(ptr);
        auto addr = static_cast<char*>(ptr) - 8;

        auto s_head = true;
        auto s_tail = true;
        for (auto i = 0u; i < 8u; ++i) {
            s_head = s_head && (addr[0       + i] == '[');
            s_tail = s_tail && (addr[size -1 - i] == ']');
        }
        if (!s_head || !s_tail) {
            printf("[!!]  mem: %p\n", ptr);
        }
        else {
            for (auto i = 0u; i < 8; ++i) {
                addr[i]             = '(';
                addr[size -1 - i]   = ')';
            }
            ::free(addr);
        }
    }
    else {
        ::free(ptr);
    }
}

NMS_API void _mzero(void* ptr, u64 size) {
    ::memset(ptr, 0, size);
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
    const auto addr = _mem_debug() ? static_cast<const char*>(ptr) - 8 : ptr;

#if     defined(NMS_OS_WINDOWS)
    const auto mem_size = ::_msize(const_cast<void*>(addr));
#elif   defined(NMS_OS_APPLE)
    const auto mem_size = ::malloc_size(addr);
#elif   defined(NMS_OS_UNIX)
    const auto mem_size = ::malloc_usable_size(const_cast<void*>(addr));
#endif
    return mem_size;
}

NMS_API void* _anew (u64 size, u64 align) {
#ifdef NMS_OS_WINDOWS
    auto ptr = ::_aligned_malloc(size, align);
#else
    void* ptr = nullptr;
    ::posix_memalign(&ptr, align, size);
#endif
    return ptr;
}

NMS_API void  _adel (void* ptr) {
#ifdef NMS_OS_WINDOWS
    ::_aligned_free(ptr);
#else
    ::free(ptr);
#endif
}

NMS_API void* vnew(int fid, u64 size) {
    ::mmap(nullptr, size, PROT_READ | PROT_WRITE | SEC_RESERVE, MAP_SHARED, fid, 0);
    return nullptr;
}

NMS_API void vdel(void* ptr, u64 size) {
    ::munmap(ptr, size);
}

NMS_API void vuse(void* base, u64 size) {
    (void)base;
    (void)size;
#ifdef NMS_OS_WINDOWS
    ::muse(base, size);
#endif
}

NMS_API void vunuse(void* base, u64 size) {
    (void)base;
    (void)size;
#ifdef NMS_OS_WINDOWS
    ::munuse(base, size);
#endif
}

}

