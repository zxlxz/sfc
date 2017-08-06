
#include <nms/thread.h>
#ifdef NMS_OS_POSIX
#include <pthread.h>
#endif

#ifdef NMS_OS_WINDOWS
using namespace nms;
extern "C" {
    void InitializeSRWLock(void* mutex);
    void AcquireSRWLockExclusive(void* mutex);
    byte TryAcquireSRWLockExclusive(void* mutex);
    void ReleaseSRWLockExclusive(void* mutex);
}
#endif

namespace nms::thread
{

Mutex::Mutex()  {
#if defined(NMS_OS_POSIX)
    static_assert(sizeof(pthread_mutex_t) <= sizeof(impl_), "nms::thread::Mutex is too small.");
    auto stat = pthread_mutex_init(reinterpret_cast<pthread_mutex_t*>(impl_), nullptr);
    (void)stat;
#elif defined(NMS_OS_WINDOWS)
    InitializeSRWLock(&impl_);
#else
#error "not impl yet"
#endif
}

Mutex::~Mutex() {
#if defined(NMS_OS_POSIX)
    pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t*>(impl_));
#elif defined(NMS_OS_WINDOWS)
#else
#error "not impl yet"
#endif
}

void Mutex::lock() noexcept {
#if defined(NMS_OS_POSIX)
    pthread_mutex_lock(reinterpret_cast<pthread_mutex_t*>(impl_));
#elif defined(NMS_OS_WINDOWS)
    AcquireSRWLockExclusive(&impl_);
#else
#error "not impl yet"
#endif
}

bool Mutex::trylock() {
#if defined(NMS_OS_POSIX)
    const auto ret = pthread_mutex_trylock(reinterpret_cast<pthread_mutex_t*>(impl_));
#elif defined(NMS_OS_WINDOWS)
    const auto ret = TryAcquireSRWLockExclusive(&impl_);
#else
#error "not impl yet"
#endif
    return ret == 0;
}

void Mutex::unlock() noexcept {
#if defined(NMS_OS_POSIX)
    pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t*>(impl_));
#elif defined(NMS_OS_WINDOWS)
    ReleaseSRWLockExclusive(&impl_);
#else
#error "not impl yet"
#endif

}

}
