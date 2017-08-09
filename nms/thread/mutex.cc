#include <nms/thread.h>

using namespace nms;

#ifdef NMS_OS_WINDOWS
extern "C" {
    void InitializeSRWLock(void* mutex);
    void AcquireSRWLockExclusive(void* mutex);
    byte TryAcquireSRWLockExclusive(void* mutex);
    void ReleaseSRWLockExclusive(void* mutex);

    static int mtx_init(mtx_t* mutex, int type) {
        InitializeSRWLock(mutex);
    }
    
    static void mtx_destroy(mtx_t* mutex) {
        (void)mutex;
    }

    static int mtx_lock(mtx_t* mutex) {
        AcquireSRWLockExclusive(impl_);
        return 0;
    } 

    static int mtx_trylock(mtx_t* mutex) {
        return TryAcquireSRWLockExclusive(&impl_);
    }

    static int mtx_destroy(mtx_t* mutex) {
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

    static void mtx_destroy(mtx_t* mutex) {
        pthread_mutex_destroy(mutex);
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

Mutex::Mutex()  {
    mtx_init(&impl_, 0);
}

Mutex::~Mutex() {
    mtx_destroy(&impl_);
}

void Mutex::lock() noexcept {
    mtx_lock(&impl_);
}

bool Mutex::trylock() {
    return mtx_trylock(&impl_)== 0;
}

void Mutex::unlock() noexcept {
    mtx_destroy(&impl_);
}

}
