#pragma once

#include <ustd/core.h>

namespace ustd::time
{

struct Duration
{
    Duration(u64 secs, u32 nanos=0)
        : _secs{ secs }, _nanos{ nanos } {
    }

    static fn from_secs(u64 secs) noexcept {
        return Duration{ secs, 0 };
    }

    static fn from_millis(u64 millis) noexcept {
        return Duration{ millis / 1000ull, millis % 1000u * 1000000u };
    }

    fn as_secs() const noexcept-> u64 {
        return _secs;
    }

    fn subsec_nanos() const noexcept-> u32 {
        return _nanos;
    }

   fn sfmt(string::String& outbuf, const fmt::Formatter& spec) const -> void;

   fn operator+(Duration rhs) const noexcept-> Duration;
   fn operator-(Duration rhs) const noexcept-> Duration;

   fn operator==(const Duration& rhs) const noexcept -> bool {
       return _secs == rhs._secs && _nanos == rhs._nanos;
   }

   fn operator!=(const Duration& rhs) const noexcept -> bool {
       return _secs != rhs._secs || _nanos != rhs._nanos;
   }

   fn operator<(const Duration& rhs) const noexcept -> bool {
       return _secs < rhs._secs || (_secs == rhs._secs && _nanos < rhs._secs);
   }

   fn operator>(const Duration& rhs) const noexcept -> bool {
       return _secs > rhs._secs || (_secs == rhs._secs && _nanos > rhs._secs);
   }

   fn operator<=(const Duration& rhs) const noexcept -> bool {
       return _secs < rhs._secs || (_secs == rhs._secs && _nanos <= rhs._secs);
   }

   fn operator>=(const Duration& rhs) const noexcept -> bool {
       return _secs > rhs._secs || (_secs == rhs._secs && _nanos >= rhs._secs);
   }
private:
    u64 _secs;
    u32 _nanos;
};

struct Instant
{
    static fn now() -> Instant;

    fn elapsed() const -> Duration;
    fn duration_since(const Instant& earlier) const->Duration;

private:
    u64 _secs;
    u32 _nanos;

    Instant(u64 secs, u32 nanos) noexcept
        : _secs{ secs }, _nanos{ nanos }
    {}
};

struct SystemTime
{
    static fn now()->SystemTime;

    fn elapsed() const->Duration;
    fn duration_since(const SystemTime& earlier) const->Duration;

private:
    u64 _secs;
    u32 _nanos;

    SystemTime(u64 secs, u32 nanos) noexcept
        : _secs{ secs }, _nanos{ nanos }
    {}
};

}
