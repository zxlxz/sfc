#pragma once

#include <ustd/core.h>

namespace ustd::time
{

struct Duration
{
    i64 _secs;
    i32 _nanos;

    Duration(u64 secs, u32 nanos=0)
        : _secs{ i64(secs) }, _nanos{ i32(nanos) } {
    }

    static fn from_secs(u64 secs) {
        return Duration{ secs, 0 };
    }

    static fn from_millis(u64 millis) {
        return Duration{ millis / 1000, millis % 1000 * 1000000 };
    }

   static fn since_monotonic() ->Duration;
   static fn since_proc_start()->Duration;

    fn as_secs() const -> u64 {
        return u64(_secs);
    }

    fn subsec_nanos() const -> u32 {
        return u32(_nanos);
    }

   fn sfmt(string::String& outbuf, const fmt::Formatter& spec) const -> void;
};

struct SystemTime
{
    i64 _secs;
    i64 _nanos;

   static fn now()->SystemTime;

   fn elapsed() const->Duration;
   fn duration_since(const SystemTime& earlier) const->Duration;
};

}
