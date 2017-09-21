#pragma once

#include <nms/core.h>
#include <nms/thread/semaphore.h>

namespace nms::thread
{

class Thread final
{
public:
#ifdef NMS_OS_UNIX
    using   thrd_ret_t = void*;
#else
    using   thrd_ret_t = void;
#endif
    constexpr static auto $buff_size = 64;

    template<class Tfunc>
    explicit Thread(Tfunc&& func) {
        using Tobj = Tvalue<Tmutable<Tfunc>>;
        static_assert(sizeof(Tobj) <= sizeof(buff_), "nms::thread::Thread: `func` size is to large");

        auto pobj = new(buff_)Tobj(fwd<Tobj>(func));

        auto pfun = [](void* ptr) -> thrd_ret_t {
            Tobj*   pobj = static_cast<Tobj*>(ptr);
            Tobj    obj(static_cast<Tobj&&>(*pobj));
            obj();

        #ifdef NMS_OS_UNIX
            return nullptr;
        #endif
        };

        start(pfun, pobj);
    }

    virtual ~Thread() {
        detach();
    }

    Thread(Thread&&) = delete;
    Thread(const Thread&) = delete;

    Thread& operator=(Thread&&)         = delete;
    Thread& operator=(const Thread&)    = delete;

public:
    NMS_API int join();
    NMS_API int detach();

    NMS_API static void yield();
    NMS_API static int  sleep(double duration);

protected:
    thrd_t  impl_   = thrd_t(0);
    u64     buff_[$buff_size/sizeof(u64)];

    NMS_API void start(thrd_ret_t(*pfun)(void*), void* pobj);
};

inline void yield() {
    Thread::yield();
}

inline void sleep(double seconds) {
    Thread::sleep(seconds);
}

}
