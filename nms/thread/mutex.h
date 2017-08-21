#pragma once

#include <nms/core.h>

namespace nms::thread
{

class Thread;
class Mutex;
class CondVar;

class Mutex final : public INocopyable
{
public:
    NMS_API Mutex();
    NMS_API ~Mutex();

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
        : mutex_(mutex) {
        auto stat = mutex_.trylock();
        if (!stat) {
            mutex_.lock();
        }
    }

    ~LockGuard() {
        mutex_.unlock();
    }
private:
    Mutex&  mutex_;
};

}

