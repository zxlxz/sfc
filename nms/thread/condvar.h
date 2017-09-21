#pragma once

#include <nms/core.h>

namespace nms::thread
{

class Mutex;

class CondVar
{
public:
    CondVar()
        : impl_{}
    {
        init();
    }

    ~CondVar() {
        destroy();
    }

    CondVar(CondVar&& rhs) {
        nms::swap(impl_, rhs.impl_);
    }

    CondVar& operator= (CondVar&& rhs) {
        if (this != &rhs) {
            nms::swap(impl_, rhs.impl_);
        }
        return *this;
    }

    CondVar(const CondVar&) = delete;
    CondVar& operator= (const CondVar&) = delete;

    NMS_API int init();
    NMS_API int destroy();
    NMS_API int signal()       noexcept;
    NMS_API int broadcast()    noexcept;
    NMS_API int wait(Mutex& lock);

private:
    cnd_t impl_;
};

}

