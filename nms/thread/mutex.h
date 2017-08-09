#pragma once

#include <nms/core.h>

namespace nms::thread
{

#ifndef NMS_BUILD
#ifdef NMS_OS_WINDOWS
#   using mtx_t = u32[1];
#else
#   using mtx_t = u32[8];
#endif
#endif

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
    explicit LockGuard(Mutex& lock)
        : lock_(lock) {
        lock_.lock();
    }

    ~LockGuard() {
        lock_.unlock();
    }
private:
    Mutex&   lock_;
};

}
