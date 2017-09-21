#pragma once

#include <nms/core.h>

namespace nms::thread
{

class Thread;
class Mutex;
class CondVar;

class Mutex final
{
public:
    Mutex() {
        init();
    }

    ~Mutex() {
        destroy();
    }

    Mutex(Mutex&& rhs)  = delete;
    Mutex(const Mutex&) = delete;

    Mutex& operator=(Mutex&&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    NMS_API int  init();
    NMS_API int  destroy();

    NMS_API void lock()     noexcept;
    NMS_API void unlock()   noexcept;
    NMS_API bool trylock();

private:
    friend class CondVar;
    mtx_t impl_ = {};
};

struct LockGuard final
{
public:
    explicit LockGuard(Mutex& mutex)
        : mutex_(&mutex) {
        mutex_->lock();
    }

    ~LockGuard() {
        mutex_->unlock();
    }

    LockGuard(LockGuard&&) = delete;
    LockGuard(const LockGuard&) = delete;

    LockGuard& operator=(LockGuard&&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
private:
    Mutex* mutex_;
};

}

