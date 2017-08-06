#include <nms/thread.h>
#include <nms/util/system.h>
#include <nms/io/log.h>

#ifdef NMS_OS_POSIX
#   include <pthread.h>
#endif

extern "C" {
    using namespace nms;
    int CloseHandle(void* handle);
    int WaitForSingleObjectEx(void* hHandle, u32 dwMilliseconds, int bAlertable);
    int SwitchToThread();
    void RaiseException(u32 dwExceptionCode, u32 dwExceptionFlags, u32 nNumberOfArguments, const void* lpArguments);
}


namespace nms::thread
{

#if defined(NMS_OS_POSIX)
static void*  _thread_callback(void* raw) {
#elif defined(NMS_OS_WINDOWS)
static unsigned __stdcall _thread_callback(void* raw) {
#endif
    auto pfun = static_cast<delegate<void()>*>(raw);

    if (pfun != nullptr) {
        try {
            (*pfun)();
        }
        catch (...) {
        }
    }

#if defined(NMS_OS_POSIX)
    return nullptr;
#elif defined(NMS_OS_WINDOWS)
    return 0;
#endif
}

NMS_API void Thread::start() {
    idx_ = 0;

    if (!static_cast<bool>(*fun_)) {
        io::log::warn("nms.thread.Thread.start: fun_[{}] is empty.", fun_);
    }
    else {
#if defined(NMS_OS_POSIX)
        auto ret = pthread_create((pthread_t*)&obj_, nullptr, _thread_callback, fun_);
#elif defined(NMS_OS_WINDOWS)
        auto obj = _beginthreadex(nullptr, 0u, &_thread_callback, fun_, 0, &idx_);
        obj_ = reinterpret_cast<void*>(obj);
#endif
        if (obj_ == nullptr || idx_ == 0) {
            io::log::error("nms.thread.Thread.start: start thread failed.");
        }
    }
}

NMS_API void Thread::detach() {
    if (obj_ == nullptr) {
        return;
    }
#if defined(NMS_OS_POSIX)
    auto ret = pthread_detach(pthread_t(obj_));
#elif defined(NMS_OS_WINDOWS)
    auto ret = CloseHandle(obj_);
    obj_ = nullptr;
#endif
    (void)ret;
}

NMS_API bool Thread::join() {
    if (obj_ == nullptr) {
        return true;
    }

#if defined(NMS_OS_POSIX)
    const auto ret = ::pthread_join(pthread_t(obj_), 0);
#elif defined(NMS_OS_WINDOWS)
    const auto ret = WaitForSingleObjectEx(obj_, 0 - 1u, 0);
#endif

    if (ret != 0) {
        return false;
    }
    detach();
    return true;
}

NMS_API void Thread::yield() {
#if defined(NMS_OS_POSIX)
    const auto ret = sched_yield();
#elif defined(NMS_OS_WINDOWS)
    const auto ret = SwitchToThread();
#endif
    (void)ret;
}

NMS_API void Thread::sleep(double duration) {
    system::sleep(duration);
}

NMS_API void Thread::setName(StrView name) {
#ifdef NMS_OS_WINDOWS
#pragma pack(push, 8)
    struct THREADNAME_INFO
    {
        u32         dwType;     // Must be 0x1000.
        const char* szName;     // Pointer to name (in user addr space).
        u32         dwThreadID; // Thread ID (-1=caller thread).
        u32         dwFlags;    // Reserved for future use, must be zero.
    };
#pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name.data();
    info.dwThreadID = idx_;
    info.dwFlags = 0;

#pragma warning(push)
#pragma warning(disable: 6320 6312 6322)
    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(u64), &info);
    }
    __except (-1) {
    }
#pragma warning(pop)
#endif

}

}

