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

    NMS_API int signal()       noexcept;
    NMS_API int broadcast()    noexcept;
    NMS_API int wait(Mutex& lock);

private:
    cnd_t impl_;
};

}

