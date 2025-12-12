#pragma once
#if defined(__unix__) || defined(__APPLE__)

#include <pthread.h>
#include <time.h>

namespace sfc::sys::thread {

using timespec_t = struct ::timespec;

using ret_t = void*;
using thrd_t = pthread_t;

inline auto current() -> thrd_t {
  return ::pthread_self();
}

inline auto start(size_t stack_size, void* (*func)(void*), void* data) -> thrd_t {
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

inline auto join(thrd_t thrd) -> bool {
  const auto err = ::pthread_join(thrd, nullptr);
  return err == 0;
}

inline auto detach(thrd_t thrd) -> bool {
  const auto err = ::pthread_detach(thrd);
  return err == 0;
}

inline void yield() {
  ::sched_yield();
}

template <size_t N>
inline auto get_name(thrd_t thrd, char (&buf)[N]) -> const char* {
  const auto err = ::pthread_getname_np(thrd, buf, N);
  if (err != 0) {
    return "";
  }
  return buf;
}

inline auto set_name(const char* name) -> bool {
#ifdef __APPLE__
  const auto err = ::pthread_setname_np(name);
#else
  const auto err = ::pthread_setname_np(pthread_self(), name);
#endif
  return err == 0;
}

inline auto sleep_ms(unsigned millis) -> bool {
  static constexpr auto MILLIS_PER_SEC = 1000U;
  static constexpr auto NANOS_PER_MILLI = 1000000U;

  const auto ts = timespec_t{
      .tv_sec = millis / MILLIS_PER_SEC,
      .tv_nsec = (millis % MILLIS_PER_SEC) * NANOS_PER_MILLI,
  };

  const auto ret = ::nanosleep(&ts, nullptr);
  return ret != -1;
}

}  // namespace sfc::sys::thread
#endif
