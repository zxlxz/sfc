#include "rc.inl"

#include "rc/time.h"
#include "rc/sys.h"

namespace rc::time {

#pragma region Duration
pub auto Duration::from_secs(u64 secs) noexcept -> Duration {
  return Duration{secs, 0};
}

pub auto Duration::from_millis(u64 millis) noexcept -> Duration {
  const auto s = millis / MILLIS_PER_SEC;
  const auto n = millis % MILLIS_PER_SEC * NANOS_PER_MILLIS;
  return {s, u32(n)};
}

pub auto Duration::from_micros(u64 micros) noexcept -> Duration {
  const auto s = micros / MICROS_PER_SEC;
  const auto n = micros % MICROS_PER_SEC * NANOS_PER_MICROS;
  return {s, u32(n)};
}

pub auto Duration::from_nanos(u64 nanos) noexcept -> Duration {
  const auto s = nanos / NANOS_PER_SEC;
  const auto n = nanos % NANOS_PER_SEC;
  return {s, u32(n)};
}

pub auto Duration::total_nanos() const noexcept -> u64 {
  return _secs * NANOS_PER_SEC + _nanos;
}

pub auto Duration::as_secs_f64() const noexcept -> f64 {
  return f64(_secs) + f64(_nanos) * 1e-9;
}

pub auto Duration::as_millis_f64() const noexcept -> f64 {
  return f64(_secs) * 1e3 + f64(_nanos) * 1e-6;
}

#pragma endregion

#pragma region Instant

pub auto Instant::now() noexcept -> Instant {
  const auto res = sys::time::get_instant_now();
  return res;
}

pub auto Instant::from_nanos(u64 nanos) noexcept -> Instant {
  return Instant{nanos / NANOS_PER_SEC, u32(nanos % NANOS_PER_SEC)};
}

pub auto Instant::total_nanos() const noexcept -> u64 {
  return _secs * NANOS_PER_SEC + _nanos;
}

pub auto Instant::as_secs_f64() const noexcept -> f64 {
  return f64(_secs) + f64(_nanos) / f64(NANOS_PER_SEC);
}

pub auto Instant::duration_since(const Instant& earlier) const -> Duration {
  const auto ns = this->total_nanos() - earlier.total_nanos();
  return Duration::from_nanos(ns);
}

pub auto Instant::elpased() const -> Duration {
  const auto t = Instant::now();
  return t.duration_since(*this);
}

auto Instant::operator+(const Duration& dur) const noexcept -> Instant {
  const auto ns = this->total_nanos() + dur.total_nanos();
  return Instant::from_nanos(ns);
}

auto Instant::operator-(const Duration& dur) const noexcept -> Instant {
  const auto ns = this->total_nanos() - dur.total_nanos();
  return Instant::from_nanos(ns);
}
#pragma endregion

#pragma region SystemTime

pub auto SystemTime::now() noexcept -> SystemTime {
  const auto res = sys::time::get_system_now();
  return res;
}

pub auto SystemTime::from_nanos(u64 nanos) noexcept -> SystemTime {
  return SystemTime{nanos / NANOS_PER_SEC, u32(nanos % NANOS_PER_SEC)};
}

pub auto SystemTime::total_nanos() const noexcept -> u64 {
  return _secs * NANOS_PER_SEC + _nanos;
}

pub auto SystemTime::duration_since(const SystemTime& earlier) const noexcept
    -> Duration {
  const auto ns = this->total_nanos() - earlier.total_nanos();
  return Duration::from_nanos(ns);
}

pub auto SystemTime::elpased() const noexcept -> Duration {
  const auto t = SystemTime::now();
  return t.duration_since(*this);
}

auto SystemTime::operator+(const Duration& dur) const noexcept -> SystemTime {
  const auto ns = this->total_nanos() + dur.total_nanos();
  return SystemTime::from_nanos(ns);
}

auto SystemTime::operator-(const Duration& dur) const noexcept -> SystemTime {
  const auto ns = this->total_nanos() + dur.total_nanos();
  return SystemTime::from_nanos(ns);
}

#pragma endregion

}  // namespace rc::time
