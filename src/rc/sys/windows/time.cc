#include "rc/sys/windows.inl"

#include "rc/time.h"

namespace rc::sys::windows::time {

static auto performance_freq() -> u64 {
  static ::LARGE_INTEGER freq = {};

  if (freq.QuadPart == 0) {
    (void)QueryPerformanceFrequency(&freq);
  }
  return u64(freq.QuadPart);
}

static auto performance_cnt() -> u64 {
  ::LARGE_INTEGER res = {};
  (void)::QueryPerformanceCounter(&res);

  return u64(res.QuadPart);
}

static auto file_time() -> u64 {
  static constexpr usize NANOS_PER_INTERVAL = 100;

  ::FILETIME t = {};
  ::GetSystemTimeAsFileTime(&t);
  const auto intervals = u64(t.dwLowDateTime) | (u64(t.dwHighDateTime) << 32);

  const auto nanos = intervals * NANOS_PER_INTERVAL;
  return nanos;
}

auto get_instant_now() -> Instant {
  static const auto freq = sys::time::performance_freq();

  const auto cnt = sys::time::performance_cnt();
  const auto nanos = cnt * NANOS_PER_SEC / freq;
  return Instant::from_nanos(nanos);
}

auto get_system_now() -> SystemTime {
  const auto nanos = sys::time::file_time();
  return SystemTime::from_nanos(nanos);
}

}  // namespace rc::sys::windows::time

