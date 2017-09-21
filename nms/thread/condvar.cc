
#include <nms/thread/condvar.h>
#include <nms/thread/mutex.h>

#ifdef NMS_OS_WINDOWS
extern "C" {
    using namespace nms;
    void InitializeConditionVariable(void*);
    void WakeConditionVariable(void*);
    void WakeAllConditionVariable(void*);
    int  SleepConditionVariableSRW(void*, void* lock, u32 ms, ulong flags);

    static int cnd_init(cnd_t* cond){
        InitializeConditionVariable(cond);
        return 0;
    }

    static int cnd_destroy(cnd_t* /*cond*/) {
        return 0;
    }

    static int cnd_signal(cnd_t* cond) {
        WakeConditionVariable(cond);
        return 0;
    }

    static int  cnd_brodcast(cnd_t* cond){
        WakeAllConditionVariable(cond);
        return 0;
    }

    static int cnd_wait(cnd_t* cond, mtx_t* mutex) {
        const auto eid  = SleepConditionVariableSRW(cond, mutex, 0xFFFFFFFFu, 0);
        return eid;
    }
}
#else
extern "C" {
    static int cnd_init(cnd_t* cond){
        return pthread_cond_init(cond, nullptr);
    }

    static int cnd_destroy(cnd_t* cond) {
        return pthread_cond_destroy(cond);
    }

    static int cnd_signal(cnd_t* cond) {
        return pthread_cond_signal(cond);
    }

    static int  cnd_brodcast(cnd_t* cond){
        return pthread_cond_broadcast(cond);
    }

    static int cnd_wait(cnd_t* cond, mtx_t* mutex) {
        return pthread_cond_wait(cond, mutex);
    }
}
#endif


namespace nms::thread
{

NMS_API int CondVar::init() {
    return cnd_init(&impl_);
}

NMS_API int CondVar::destroy() {
    return cnd_destroy(&impl_);
}

NMS_API int CondVar::signal() noexcept {
    return cnd_signal(&impl_);
}

NMS_API int CondVar::broadcast() noexcept {
    return cnd_brodcast(&impl_);
}

NMS_API int CondVar::wait(Mutex& mutex) {
    return cnd_wait(&impl_, &mutex.impl_);
}

}
