#include <nms/thread.h>
#include <nms/util/system.h>
#include <nms/io/log.h>

#ifdef NMS_OS_WINDOWS
extern "C" {
    using namespace nms;
    typedef int(__stdcall *thrd_start_t)(void*);

    int CloseHandle(void* handle);
    int WaitForSingleObjectEx(void* hHandle, u32 dwMilliseconds, int bAlertable);
    int SwitchToThread();
    void RaiseException(u32 dwExceptionCode, u32 dwExceptionFlags, u32 nNumberOfArguments, const void* lpArguments);

    static int thrd_create(thrd_t* thr, thrd_start_t func, void* arg) {
        u32  tid = 0;
        auto obj = _beginthreadex(nullptr, 0u, func, nullptr, 0, &tid);
        thr = reinterpret_cast<void*>(obj);
        return int(tid);
    }

    static int thrd_detach(thrd_t thr) {
        ::CloseHandle(thr);
        return 0;
    }

    static int thrd_join(thrd_t thr, int* res) {
        ::WaitForSingleObject(thr, 0xFFFFFFFF, 0);
    }

    static void thrd_yeild() {
        ::SwitchToThread();
    }
}
#else

extern "C" {
    typedef void*(thrd_start_t)(void*);

    static int thrd_create(thrd_t* thr, thrd_start_t func, void* arg) {
        return pthread_create(thr, nullptr, func, arg);
    }

    static int thrd_detach(thrd_t thr) {
        return pthread_detach(thr);
    }

    static int thrd_join(thrd_t thr, int* res) {
        (void)res;
        return pthread_join(thr, nullptr);
    }

    static void thrd_yeild() {
        sched_yield();
    }
}
#endif


namespace nms::thread
{

#ifdef NMS_OS_WINDOWS
static unsigned __stdcall _thread_callback(void* raw){
#else
static void*  _thread_callback(void* raw)
#endif
{
    auto pfun = static_cast<delegate<void()>*>(raw);

    if (pfun != nullptr) {
        try {
            (*pfun)();
        }
        catch (...) {
        }
    }
    return 0;
}

NMS_API void Thread::start() {
    impl_ = nullptr;
    idx_  = 0;

    if (!bool(*func_)) {
        io::log::warn("nms.thread.Thread.start: fun_[{}] is empty.", func_);
    }
    else {
        auto ret = thrd_create(&impl_, &_thread_callback, func_);
#ifdef NMS_OS_WINDOWS
        idx_ = ret;
#endif
        if (impl_ == nullptr) {
            io::log::error("nms.thread.Thread.start: start thread failed.");
        }
    }
}

NMS_API int Thread::detach() {
    if (impl_ == nullptr) {
        return 0;
    }
    auto ret = thrd_detach(impl_);
    impl_ = nullptr;
    return ret;
}

NMS_API int Thread::join() {
    if (impl_ == nullptr) {
        return 0;
    }
    return thrd_join(impl_, nullptr);
}

NMS_API void Thread::yield() {
    thrd_yeild();
}

NMS_API int Thread::sleep(double duration) {
    system::sleep(duration);
    return 0;
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

