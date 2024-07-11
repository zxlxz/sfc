#include "system.h"

#include "sfc/sys/time.inl"

namespace sfc::time {

namespace sys_imp = sys::time;

auto System::now() -> System {
  const auto imp = sys_imp::System::now();
  return System{static_cast<u64>(imp.secs), static_cast<u64>(imp.nanos)};
}

auto System::elpased() const -> Duration {
  const auto now = System::now();
  const auto res = now - *this;
  return res;
}

auto System::duration_since(const System& earlier) const -> Duration {
  const auto res = *this - earlier;
  return res;
}

auto System::operator==(const System& other) const -> bool {
  return _secs == other._secs && _nanos == other._nanos;
}

auto System::operator<(const System& other) const -> bool {
  return _secs < other._secs || (_secs == other._secs && _secs < other._secs);
}

auto System::operator<=(const System& other) const -> bool {
  return _secs < other._secs || (_secs == other._secs && _secs <= other._secs);
}

auto System::operator-(const System& rhs) const -> Duration {
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

auto System::operator+(const Duration& rhs) const -> System {
  auto secs = num::saturating_add(_secs, rhs._secs);
  auto nanos = _nanos + rhs._nanos;
  if (nanos > NANOS_PER_SEC) {
    nanos -= NANOS_PER_SEC;
    secs = num::saturating_add<u64>(secs, 1U);
  }
  return {secs, nanos};
}

auto System::operator-(const Duration& rhs) const -> System {
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

void System::operator+=(const Duration& dur) {
  *this = *this + dur;
}

void System::operator-=(const Duration& dur) {
  *this = *this - dur;
}

}  // namespace sfc::time
