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
        : fun_(new delegate<void()>(fwd<F>(f)))
    {
        start();
    }

    Thread(Thread&& t) noexcept
        : fun_(move(t.fun_))
        , obj_(move(t.obj_))
        , idx_(move(t.idx_))
    {
        t.fun_ = {};
        t.obj_ = nullptr;
        t.idx_  = 0;
    }

    ~Thread() {
        detach();
        delete fun_;
    }

    Thread& operator=(Thread&& rhs) noexcept {
        nms::swap(fun_, rhs.fun_);
        nms::swap(obj_, rhs.obj_);
        nms::swap(idx_, rhs.idx_);
        return *this;
    }

    NMS_API void setName(StrView name);
    NMS_API bool join();
    NMS_API void detach();

    NMS_API static void yield();
    NMS_API static void sleep(double duration);

private:
    delegate<void()>*   fun_ = nullptr;
    void*               obj_ = nullptr;
    u32                 idx_ = 0;
    NMS_API void start();
};

}