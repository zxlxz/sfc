#pragma once

#include <nms/core.h>

namespace nms::thread
{

class  Semaphore final : INocopyable
{
public:
    NMS_API explicit Semaphore();
    NMS_API explicit Semaphore(u32 value);
    NMS_API ~Semaphore();

    Semaphore(Semaphore&& rhs) noexcept
        : impl_(rhs.impl_) 
    {
        rhs.impl_ = nullptr;
    }

    Semaphore& operator==(Semaphore&& rhs) noexcept {
        nms::swap(impl_, rhs.impl_);
        return *this;
    }

    NMS_API Semaphore& operator+=(u32 val);
    NMS_API Semaphore& operator++();
    NMS_API Semaphore& operator--();

private:
    void*   impl_ = nullptr;
};

}