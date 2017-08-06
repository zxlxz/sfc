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
#ifdef _WIN32
    u64 impl_[1];
#else
    u64 impl_[8];     // 64 byte is enough
#endif
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
