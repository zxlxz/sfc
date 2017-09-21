#pragma once

#include <nms/core.h>

namespace nms::thread
{

class  Semaphore final
{
public:
    Semaphore() {
        open(0);
    }

    explicit Semaphore(u32 value) {
        open(value);
    }

    ~Semaphore() {
        close();
    }

    Semaphore(Semaphore&& rhs) noexcept
        : impl_(rhs.impl_)
    {
        rhs.impl_ = nullptr;
    }

    Semaphore& operator=(Semaphore&& rhs) {
        if (&rhs != this) {
            close();
            impl_ = rhs.impl_;
            rhs.impl_ = nullptr;
        }
        return *this;
    }
    Semaphore(const Semaphore&) = delete;


    Semaphore& operator=(const Semaphore&) = delete;

    NMS_API void open(u32 value);
    NMS_API void close();

    NMS_API Semaphore& operator+=(u32 val);
    NMS_API Semaphore& operator++();
    NMS_API Semaphore& operator--();

private:
    sem_t* impl_ = nullptr;
};

}