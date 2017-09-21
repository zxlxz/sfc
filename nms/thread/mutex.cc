#include <nms/test.h>
#include <nms/thread/mutex.h>

using namespace nms;

#ifdef NMS_OS_WINDOWS
extern "C" {
    void InitializeSRWLock(void* mutex);
    void AcquireSRWLockExclusive(void* mutex);
    byte TryAcquireSRWLockExclusive(void* mutex);
    void ReleaseSRWLockExclusive(void* mutex);

    static int mtx_init(mtx_t* mutex, int type) {
        (void)type;
        InitializeSRWLock(mutex);
        return 0;
    }

    static int mtx_destroy(mtx_t* mutex) {
        (void)mutex;
        return 0;
    }

    static int mtx_lock(mtx_t* mutex) {
        AcquireSRWLockExclusive(mutex);
        return 0;
    }

    static int mtx_trylock(mtx_t* mutex) {
        return TryAcquireSRWLockExclusive(mutex);
    }

    static int mtx_unlock(mtx_t* mutex) {
        ReleaseSRWLockExclusive(mutex);
        return 0;
    }
}
#else
extern "C" {
    static int mtx_init(mtx_t* mutex, int type) {
        (void)type;
        return pthread_mutex_init(mutex, nullptr);
    }

    static int mtx_destroy(mtx_t* mutex) {
        return pthread_mutex_destroy(mutex);
    }

    static int mtx_lock(mtx_t* mutex) {
        return pthread_mutex_lock(mutex);
    }

    static int mtx_trylock(mtx_t* mutex){
        return pthread_mutex_trylock(mutex);
    }

    static int mtx_unlock(mtx_t* mutex) {
        return pthread_mutex_unlock(mutex);
    }
}
#endif

namespace nms::thread
{

NMS_API int Mutex::init()  {
    const auto stat = mtx_init(&impl_, 0);
    if (stat != 0) {
        io::log::error("nms.thread.Mutex: init failed");
    }
    return stat;
}

NMS_API int Mutex::destroy() {
    const auto stat =  mtx_destroy(&impl_);
    if (stat != 0) {
        io::log::error("nms.thread.Mutex: destroy failed");
    }
    return stat;
}

NMS_API void Mutex::lock() noexcept {
    const auto stat = mtx_lock(&impl_);
    if (stat != 0) {
        io::log::error("nms.thread.Mutex: lock failed");
    }
}

NMS_API bool Mutex::trylock() {
    const auto stat =  mtx_trylock(&impl_);
    return stat == 0;
}

NMS_API void Mutex::unlock() noexcept {
    const auto stat = mtx_unlock(&impl_);
    if (stat != 0) {
        io::log::error("nms.thread.Mutex: unlock failed");
    }
}

}
