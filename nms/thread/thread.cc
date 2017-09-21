#include <nms/test.h>
#include <nms/thread.h>
#include <nms/util/system.h>

#ifdef NMS_OS_WINDOWS
extern "C" {
    using namespace nms;
    using thrd_start_t = _beginthread_proc_type;

    int CloseHandle(void* handle);
    int WaitForSingleObjectEx(void* hHandle, u32 dwMilliseconds, int bAlertable);
    int SwitchToThread();

    static int thrd_create(thrd_t* thr, thrd_start_t func, void* arg) {
        u32  tid = 0;
        auto obj = _beginthread(func, 0, arg);
        *thr = reinterpret_cast<thrd_t>(obj);
        return int(tid);
    }

    static int thrd_detach(thrd_t thr) {
        ::CloseHandle(thr);
        return 0;
    }

    static int thrd_join(thrd_t thr, int* /*res*/) {
        const auto ret = ::WaitForSingleObjectEx(thr, 0xFFFFFFFFu, 0);
        return ret;
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

NMS_API void Thread::start(thrd_ret_t(*pfun)(void*), void* pobj) {
    if (impl_ != thrd_t(0)) {
        return;
    }
    auto ret = thrd_create(&impl_, pfun, pobj);

    if (impl_ == thrd_t(0) && ret == 0) {
        io::log::error("nms.thread.Thread.start : start thread failed.");
    } else {
        io::log::debug("nms.thread.Thread.start : {}", impl_);
    }
}

NMS_API int Thread::detach() {
    if (impl_ == thrd_t(0) ) {
        return 0;
    }

    io::log::debug("nms.thread.Thread.detach: {}", impl_);
    auto ret = thrd_detach(impl_);
    impl_ = thrd_t(0);
    return ret;
}

NMS_API int Thread::join() {
    if (impl_ == thrd_t(0) ) {
        return 0;
    }

    io::log::debug("nms.thread.Thread.join  : {}", impl_);
    auto ret = thrd_join(impl_, nullptr);
    impl_ = thrd_t(0);
    return ret;
}

NMS_API void Thread::yield() {
    thrd_yeild();
}

NMS_API int Thread::sleep(double duration) {
    system::sleep(duration);
    return 0;
}

}

