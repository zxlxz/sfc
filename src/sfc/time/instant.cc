#include "instant.h"

#include "sfc/sys/time.inl"

namespace sfc::time {

namespace sys_imp = sys::time;

auto Instant::now() -> Instant {
  const auto imp = sys_imp::Instant::now();
  return Instant{static_cast<u64>(imp.secs), static_cast<u64>(imp.nanos)};
}

auto Instant::elpased() const -> Duration {
  const auto now = Instant::now();
  const auto res = now - *this;
  return res;
}

auto Instant::duration_since(Instant earlier) const -> Duration {
  const auto res = *this - earlier;
  return res;
}

auto Instant::operator==(const Instant& other) const -> bool {
  return _secs == other._secs && _nanos == other._nanos;
}

auto Instant::operator<(const Instant& other) const -> bool {
  return _secs < other._secs || (_secs == other._secs && _secs < other._secs);
}

auto Instant::operator<=(const Instant& other) const -> bool {
  return _secs < other._secs || (_secs == other._secs && _secs <= other._secs);
}

auto Instant::operator-(Instant rhs) const -> Duration {
  auto secs = num::saturating_sub(_secs, rhs._secs);
  auto nanos = _nanos - rhs._nanos;
  if (_nanos < rhs._nanos) {
    if (secs == 0) {
      nanos = 0;
    } else {
      secs = secs - 1;
      nanos = _nanos + NANOS_PER_SEC - rhs._nanos;
    }
  }
  return {secs, nanos};
}

auto Instant::operator+(Duration rhs) const -> Instant {
  auto secs = num::saturating_add(_secs, rhs._secs);
  auto nanos = _nanos + rhs._nanos;
  if (nanos > NANOS_PER_SEC) {
    nanos -= NANOS_PER_SEC;
    secs = num::saturating_add<u64>(secs, 1U);
  }
  return {secs, nanos};
}

auto Instant::operator-(Duration rhs) const -> Instant {
  auto secs = num::saturating_sub(_secs, rhs._secs);
  auto nanos = _nanos - rhs._nanos;
  if (_nanos < rhs._nanos) {
    if (secs == 0) {
      nanos = 0;
    } else {
      secs = secs - 1;
      nanos = _nanos + NANOS_PER_SEC - rhs._nanos;
    }
  }
  return {secs, nanos};
}

void Instant::operator+=(Duration dur) {
  *this = *this + dur;
}

void Instant::operator-=(Duration dur) {
  *this = *this - dur;
}

}  // namespace sfc::time
