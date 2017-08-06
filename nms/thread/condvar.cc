
#include <nms/thread/condvar.h>
#include <nms/thread/mutex.h>

#ifdef NMS_OS_POSIX
#   include <pthread.h>
#endif

#ifdef NMS_OS_WINDOWS
extern "C" {
    using namespace nms;
    void InitializeConditionVariable(void*);
    void WakeConditionVariable(void*);
    void WakeAllConditionVariable(void*);
    int  SleepConditionVariableSRW(void*, void* lock, u32 ms, ulong flags);
}
#endif

namespace nms::thread
{

NMS_API CondVar::CondVar() {
#if defined(NMS_OS_POSIX)
    static_assert(sizeof(pthread_mutex_t) <= sizeof(impl_), "nms::thread::CondVar is too small");
    pthread_cond_init(reinterpret_cast<pthread_cond_t*>(impl_), nullptr);
#elif defined(NMS_OS_WINDOWS)
    InitializeConditionVariable(&impl_);
#else
#   error "not impl yet"
#endif
}

NMS_API CondVar::~CondVar() {
#if defined(NMS_OS_POSIX)
    pthread_cond_destroy(reinterpret_cast<pthread_cond_t*>(impl_));
#elif defined(NMS_OS_WINDOWS)
#else
#   error "not impl yet"
#endif
}

NMS_API void CondVar::notify() noexcept {
#if defined(NMS_OS_POSIX)
    pthread_cond_signal(reinterpret_cast<pthread_cond_t*>(impl_));
#elif defined(NMS_OS_WINDOWS)
    WakeConditionVariable(&impl_);
#else
#   error "not impl yet"
#endif
}

NMS_API void CondVar::notifyAll() noexcept {
#if defined(NMS_OS_POSIX)
    pthread_cond_broadcast(reinterpret_cast<pthread_cond_t*>(impl_));
#elif defined(NMS_OS_WINDOWS)
    WakeAllConditionVariable(&impl_);
#else
#   error "not impl yet"
#endif
}

NMS_API bool CondVar::wait(Mutex& mutex, double seconds) {
#if defined(NMS_OS_POSIX)
    auto eid = 0;
    if (seconds<0) {
        eid = pthread_cond_wait(reinterpret_cast<pthread_cond_t*>(impl_), reinterpret_cast<pthread_mutex_t*>(mutex.impl_));
    }
    else {
        timespec ts = { u32(seconds), u32( (seconds-u32(seconds)) * 1e9) };
        eid = pthread_cond_timedwait(reinterpret_cast<pthread_cond_t*>(impl_), reinterpret_cast<pthread_mutex_t*>(mutex.impl_), &ts);
    }
#elif defined(NMS_OS_WINDOWS)
    const auto infinite = 0xFFFFFFFF;
    const auto msec = seconds*1e6;
    const auto time = (msec<0 || msec > infinite) ? infinite : u32(msec);
    const auto eid  = SleepConditionVariableSRW(&impl_, &mutex.impl_, time, 0);
#else
#   error "not impl yet"
#endif
    return eid == 0;
}

}
