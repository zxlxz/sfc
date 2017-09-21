#include <nms/test.h>
#include <nms/thread/semaphore.h>

#ifdef NMS_OS_WINDOWS
extern "C" {
    struct sem_t;
    using namespace nms;

    void*   CreateSemaphoreA(void*attributes, u32 lInitValue, u32 lMaximumCount, const char* name);
    int     CloseHandle(void* handle);
    int     ReleaseSemaphore(void* handle, u32 lReleaseCount, u32* lpPreviousCount);
    int     WaitForSingleObject(void* handle, u32 dwMilliseconds);

    static sem_t* sem_open(const char* name, int /*oflag*/, int /*mode*/, u32 value) {
        constexpr static auto max_cnt = 65536;
        const auto ret = CreateSemaphoreA(nullptr, value, max_cnt, name);
        return static_cast<sem_t*>(ret);
    }

    static int sem_post(sem_t* p_sem, u32 count=1) {
        u32 prev = 0;
        const auto ret = ReleaseSemaphore(p_sem, count, &prev);
        return ret;
    }

    static int sem_close(void* p_sem) {
        const auto ret = CloseHandle(p_sem);
        return ret;
    }

    static int sem_wait(sem_t* p_sem) {
        constexpr static auto max_ms = 0xFFFFFFFFu;
        const auto ret = WaitForSingleObject(p_sem, max_ms);
        return ret;
    }
}
#endif

namespace nms::thread
{

NMS_API void Semaphore::open(u32 value) {
    static auto pid  = i32(::getpid());
    static auto rand = i32(::clock());
    static auto idx = 0;

    char sem_name[64];
    const auto  count = snprintf(sem_name, sizeof(sem_name), "nms_%d.%d.%d.sem", rand, pid, idx++);
    (void)count;

    impl_ = sem_open(sem_name, O_CREAT, 0, value);
}


NMS_API void Semaphore::close() {
    if (impl_ == nullptr) {
        return;
    }

    sem_close(static_cast<sem_t*>(impl_));
    impl_ = nullptr;
}

NMS_API Semaphore& Semaphore::operator+=(u32 n) {
    if (n == 0) {
        return *this;
    }
#ifdef NMS_OS_WINDOWS
    sem_post(static_cast<sem_t*>(impl_), n);
#else
    sem_post(static_cast<sem_t*>(impl_));
#endif
    return *this;
}

NMS_API Semaphore& Semaphore::operator++() {
    sem_post(static_cast<sem_t*>(impl_));
    return *this;
}

NMS_API Semaphore& Semaphore::operator--() {
    auto stat = sem_wait(static_cast<sem_t*>(impl_));
    (void)stat;
    return *this;
}

}
