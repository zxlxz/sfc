#include "config.h"

namespace ustd::thread
{

fn Thread::id() const -> int {
    let ret = thrd_id(_handle);
    return ret;
}

fn Thread::_detach() -> void {
    let ret = thrd_detach(_handle);
    if (ret == 0) {
        _handle = nullptr;
    }
}

fn Thread::current() -> Thread {
    let handle = thrd_current();
    return Thread{ handle };
}

fn JoinHandle<void>::join() && ->Result<void> {
    using Result = Result<void>;

    using res_t = int;

    mut res = res_t{ 0 };
    let ret = thrd_join(_thr._handle, &res);

    if (ret == thrd_success) {
        _thr._handle = nullptr;
        return res == 0 ? Result::Ok() : Result::Err(thrd_error);
    }
    return Result::Err(thrd_error);
}

fn Builder::_spawn_impl(void* obj, int(*func)(void*)) -> Result<Thread> {
    using Result = Result<Thread>;

    mut thr = thrd_t{};
    let ret = thrd_create(&thr, func, obj);
    return ret == thrd_success ? Result::Ok(Thread{ thr }) : Result::Err(ret);
}

}
