#pragma once

#include <nms/core.h>

namespace nms::thread
{

class  Thread final
    : public INocopyable
{
public:
    template<class F>
    explicit Thread(F&& f)
        : func_(new delegate<void()>(fwd<F>(f)))
    {
        start();
    }

    Thread(Thread&& t) noexcept
        : func_(move(t.func_))
        , impl_(move(t.impl_))
        , idx_ (move(t.idx_))
    {
        t.func_ = {};
        t.impl_ = thrd_t(0);
        t.idx_  = 0;
    }

    ~Thread() {
        detach();
        if (func_ != nullptr) {
            delete func_;
        }
    }

    Thread& operator=(Thread&& rhs) noexcept {
        nms::swap(func_, rhs.func_);
        nms::swap(impl_, rhs.impl_);
        nms::swap(idx_,  rhs.idx_);
        return *this;
    }

    NMS_API void setName(StrView name);
    NMS_API int join();
    NMS_API int detach();

    NMS_API static void yield();
    NMS_API static int  sleep(double duration);

private:
    delegate<void()>*   func_   = nullptr;
    thrd_t              impl_   = thrd_t(0);
    u32                 idx_    = 0;
    NMS_API void start();
};

}