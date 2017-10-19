#include "config.h"

namespace ustd::thread
{

fn sleep(time::Duration dur) -> void {
    let ts_dur = timespec{ dur._secs , dur._nanos };
    mut ts_rem = timespec{};
    let ret = ::thrd_sleep(&ts_dur, &ts_rem);
    (void)ret;
}

fn sleep_ms(u32 ms) -> void {
    let secs = ms / 1000;
    let nanos = ms % 1000 * 1000000;
    let dur = time::Duration{ secs, nanos };
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
