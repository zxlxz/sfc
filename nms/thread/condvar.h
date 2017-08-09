#pragma once

#include <nms/core.h>

namespace nms::thread
{

#ifndef NMS_BUILD
#ifdef NMS_OS_WINDOWS
#   using cnd_t = u32[1];
#else
#   using cnd_t = u32[8];
#endif
#endif

class Mutex;

class CondVar : public INocopyable
{
public:
    NMS_API CondVar();
    NMS_API ~CondVar();

    NMS_API int signal()       noexcept;
    NMS_API int broadcast()    noexcept;
    NMS_API int wait(Mutex& lock);

private:
    cnd_t impl_;
};

}
