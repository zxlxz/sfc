#include "config.h"

#include "ustd/time.h"
#include "ustd/test.h"
#include "ustd/thread.h"

namespace ustd::thread
{

#pragma region thread
fn Thread::id() const -> ThreadId {
    let ret = thrd_id(_handle);
    return ret;
}

fn Thread::_detach() -> void {
    let ret = thrd_detach(_handle);
    if (ret == 0) {
        _handle = nullptr;
    }
}

fn JoinHandle<void>::join() && ->Result<void> {
    using Result = Result<void>;

    using res_t = int;

    mut res = res_t{0};
    let ret = thrd_join(_thr._handle, &res);

    if(ret == thrd_success) {
        _thr._handle = nullptr;
        return res == 0 ? Result::Ok() : Result::Err(thrd_error);
    }
    return Result::Err(thrd_error);
}

fn Thread::Builder::_spawn_impl(void* obj, int(*func)(void*)) -> Result<Thread> {
    using Result = Result<Thread>;

    mut thr = thrd_t{};
    let ret = thrd_create(&thr, func, obj);
    return ret == thrd_success ? Result::Ok(Thread{ thr }) : Result::Err(ret);
}
#pragma endregion

fn sleep(time::Duration dur) -> void {
    let ts_dur  = timespec{ dur._secs , dur._nanos };
    mut ts_rem  = timespec{};
    let ret     = ::thrd_sleep(&ts_dur, &ts_rem);
    (void)ret;
}

fn sleep_ms(u32 ms) -> void {
    let secs = ms / 1000;
    let nanos= ms % 1000 * 1000000;
    let dur  = time::Duration{ secs, nanos };
    sleep(dur);
}

[unittest(sleep)]
{
    let ms = 10;
    let t0 = time::SystemTime::now();
    sleep_ms(ms);
    let t1 = time::SystemTime::now();

    let dur = t1.duration_since(t0);
    println("\tsleep={}ms, dur={m}", ms, dur);
};

}
