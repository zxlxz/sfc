#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::thread {

using ret_t = void*;
using thrd_t = pthread_t;

inline auto thrd_current() -> thrd_t {
  return ::pthread_self();
}

inline auto thrd_create(size_t stack_size, ret_t (*func)(void*), void* data) -> thrd_t {
  // attr
  auto attr = ::pthread_attr_t{};
  ::pthread_attr_init(&attr);
  if (stack_size != 0) {
    ::pthread_attr_setstacksize(&attr, stack_size);
  }

  // create
  auto thrd = pthread_t{};
  const auto ret = ::pthread_create(&thrd, &attr, func, data);
  ::pthread_attr_destroy(&attr);

  if (ret != 0) {
    return {};
  }
  return thrd;
}

inline auto thrd_join(thrd_t thrd) -> bool {
  const auto err = ::pthread_join(thrd, nullptr);
  return err == 0;
}

inline auto thrd_detach(thrd_t thrd) -> bool {
  const auto err = ::pthread_detach(thrd);
  return err == 0;
}

inline void thrd_yield() {
  ::sched_yield();
}

template <size_t N>
inline auto thrd_name(thrd_t thrd, char (&buf)[N]) -> const char* {
  const auto err = ::pthread_getname_np(thrd, buf, N);
  if (err != 0) {
    return "";
  }
  return buf;
}

inline auto thrd_setname(const char* name) -> bool {
#ifdef __APPLE__
  const auto err = ::pthread_setname_np(name);
#else
  const auto err = ::pthread_setname_np(pthread_self(), name);
#endif
  return err == 0;
}

inline auto thrd_sleep_ms(unsigned millis) -> bool {
  static constexpr auto MILLIS_PER_SEC = 1000U;
  static constexpr auto NANOS_PER_MILLI = 1000000U;

  auto ts = ::timespec{
      .tv_sec = millis / MILLIS_PER_SEC,
      .tv_nsec = (millis % MILLIS_PER_SEC) * NANOS_PER_MILLI,
  };

  while (true) {
    auto rem = ::timespec{};
    const auto err = ::nanosleep(&ts, &rem);
    if (err == 0) {
      break;
    }
    if (errno != EINTR) {
      return false;
    }
    ts = rem;
  }
  return true;
}

}  // namespace sfc::sys::.thread
