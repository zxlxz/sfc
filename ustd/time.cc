#include "config.h"

namespace ustd::time
{

using namespace ustd::fmt;
using namespace ustd::string;

static let g_proc_start_time = Duration::since_monotonic();

fn Duration::since_monotonic() -> Duration {
    mut res = timespec{};
    let ret = clock_gettime(CLOCK_MONOTONIC, &res);
    return Duration{ u64(res.tv_sec), u32(res.tv_nsec) };
}

fn Duration::since_proc_start() -> Duration {
    let now     = since_monotonic();
    mut secs    = now._secs - g_proc_start_time._secs;
    mut nanos   = now._nanos - g_proc_start_time._nanos;

    if (nanos < 0) {
        secs  -= 1;
        nanos += 1000000000;
    }

    return Duration{u64(secs), u32(nanos)};
}

[unittest(Duration::since_proc_start)] 
{
    let dur = Duration::since_proc_start();
};


fn Duration::sfmt(String& outbuf, const Formatter& fmtspec) const -> void {
    let secs = f64(_secs) + f64(_nanos)*1e-9;
    mut spec = fmtspec;

    switch (spec.type) {
    case 's':
        if (spec.prec == 0) {
            spec.prec = 6;
        }
        spec.sformat_val(outbuf, secs);
        outbuf.push('s');
        break;
    case 'm':
        if (spec.prec == 0) {
            spec.prec = 3;
        }
        spec.sformat_val(outbuf, secs*1e3);
        outbuf.push_str("ms");
        break;
    case 'n':
        if (spec.width == 0) {
            spec.width = 1;
        }
        spec.sformat_val(outbuf, secs*1e9);
        outbuf.push_str("ns");
        break;
    default:
        if (spec.prec == 0) {
            spec.prec = 6;
        }
        spec.sformat_val(outbuf, secs);
        break;
    }
}

fn SystemTime::now()->SystemTime {
    mut current_ts = timespec{};

#ifdef _WIN32    
    ::timespec_get(&current_ts, TIME_UTC);
#else
    ::clock_gettime(CLOCK_REALTIME, &current_ts);
#endif

    return SystemTime{ current_ts.tv_sec, current_ts.tv_nsec };
}

[unittest(SystemTime::now)]
{
    
};

fn SystemTime::elapsed() const->Duration {
    let time_now = SystemTime::now();
    let duration = time_now.duration_since(*this);
    return duration;
}

[unittest(SystemTime::elapsed)]
{};

fn SystemTime::duration_since(const SystemTime& earlier) const -> Duration {
    mut secs  = _secs - earlier._secs;
    mut nanos = _nanos - earlier._nanos;

    if (nanos < 0) {
        secs  -= 1;
        nanos += 1000000000;
    }

    if (secs < 0) {
        return { 0, 0 };
    }

    return { u64(secs) , u32(nanos) };
}

}
