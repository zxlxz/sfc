#pragma once

#include <nms/core.h>

namespace nms::thread
{

class Mutex;

class CondVar : public INocopyable
{
public:
    NMS_API CondVar();
    NMS_API ~CondVar();

    NMS_API void notify()       noexcept;
    NMS_API void notifyAll()    noexcept;

    NMS_API bool wait(Mutex& lock, double sec);

    bool wait(Mutex& lock)  { return wait(lock, -1); }

private:
#ifdef _WIN32
    u64  impl_[1];
#else
    u64  impl_[8];
#endif
};

}
