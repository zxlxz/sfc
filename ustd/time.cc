#include "config.h"

namespace ustd::time
{

using namespace ustd::fmt;
using namespace ustd::string;

fn Duration::sfmt(String& outbuf, const Formatter& fmtspec) const -> void {
    let secs = f64(_secs) + f64(_nanos)*1e-9;
    mut spec = fmtspec;

    switch (spec.type) {
    case 's':
        if (spec.prec == 0) {
            spec.prec = 6;
        }
        spec.sfmt_val(outbuf, secs);
        outbuf.push('s');
        break;
    case 'm':
        if (spec.prec == 0) {
            spec.prec = 3;
        }
        spec.sfmt_val(outbuf, secs*1e3);
        outbuf.push_str("ms");
        break;
    case 'n':
        if (spec.width == 0) {
            spec.width = 1;
        }
        spec.sfmt_val(outbuf, secs*1e9);
        outbuf.push_str("ns");
        break;
    default:
        if (spec.prec == 0) {
            spec.prec = 6;
        }
        spec.sfmt_val(outbuf, secs);
        break;
    }
}

fn Duration::operator+(Duration rhs) const noexcept->Duration {
    // check overflow
    let cond    = 1000000000u - _nanos > rhs._nanos;
    let res     = cond
        ? Duration{ _secs + rhs._secs + 0, _nanos + rhs._nanos }
        : Duration{ _secs + rhs._secs + 1, _nanos + rhs._nanos - 1000000000u }
    ;

    return res;
}

fn Duration::operator-(Duration rhs) const noexcept->Duration {
    if (_secs < rhs._secs) {
        return Duration{ 0u, 0u };
    }
    if (_secs == rhs._secs && _nanos < rhs._nanos) {
        return Duration{ 0u, 0u };
    }

    // check overflow
    let cond = _nanos > rhs._nanos;
    let res = cond
        ? Duration{ _secs - rhs._secs - 0, _nanos - rhs._nanos    }
        : Duration{ _secs - rhs._secs - 1, _nanos + (1000000000 - rhs._nanos)  }
    ;

    return res;
}

fn Instant::now() -> Instant {
    mut res = timespec{};
    let ret = clock_gettime(CLOCK_MONOTONIC, &res);
    (void)ret;
    return Instant{ u64(res.tv_sec), u32(res.tv_nsec) };
}

fn Instant::elapsed() const->Duration {
    let time_now = Instant::now();
    let duration = time_now.duration_since(*this);
    return duration;
}

fn Instant::duration_since(const Instant& earlier) const -> Duration {
    let d1 = Duration(_secs, _nanos);
    let d2 = Duration(earlier._secs, earlier._nanos);
    return d1 - d2;
}

fn SystemTime::now()->SystemTime {
    mut current_ts = timespec{};
    ::clock_gettime(CLOCK_REALTIME, &current_ts);
    return SystemTime{ u64(current_ts.tv_sec), u32(current_ts.tv_nsec) };
}

fn SystemTime::elapsed() const->Duration {
    let time_now = SystemTime::now();
    let duration = time_now.duration_since(*this);
    return duration;
}

fn SystemTime::duration_since(const SystemTime& earlier) const -> Duration {
    let d1 = Duration(_secs, _nanos);
    let d2 = Duration(earlier._secs, earlier._nanos);
    return d1 - d2;
}

[unittest(SystemTime::now)]{

}

}
