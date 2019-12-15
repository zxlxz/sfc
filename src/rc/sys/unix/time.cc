#include "rc/sys/unix.inl"

namespace rc::sys::unix::time {

auto get_instant_now() -> Instant {
  struct ::timespec ts = {};
  const auto res = ::clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
  if (res == -1) {
    throw io::Error::last_os_error();
  }
  return Instant{u64(ts.tv_sec), u32(ts.tv_nsec)};
}

auto get_system_now() -> SystemTime {
  struct ::timespec ts = {};
  const auto res = ::clock_gettime(CLOCK_REALTIME, &ts);
  if (res == -1) {
    throw io::Error::last_os_error();
  }
  return SystemTime{u64(ts.tv_sec), u32(ts.tv_nsec)};
}

}  // namespace rc::sys::unix::time
