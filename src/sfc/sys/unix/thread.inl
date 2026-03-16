#pragma once

#include "sfc/sys/unix/mod.inl"
#define _SFC_SYS_THREAD_

namespace sfc::sys::unix {

using thrd_ret_t = void*;

struct Thread {
  using func_t = void* (*)(void*);
  pthread_t _thr = 0;

 public:
  static auto current() -> u32 {
#ifdef __APPLE__
    auto tid = ino_t{0};
    ::pthread_threadid_np(nullptr, &tid);
#else
    const auto tid = ::pthread_threadid_np();
#endif
    return static_cast<u32>(tid);
  }

  static auto spawn(size_t stack_size, func_t func, void* data) -> Thread {
    // thread attr
    auto attr = ::pthread_attr_t{};
    ::pthread_attr_init(&attr);
    if (stack_size != 0) {
      ::pthread_attr_setstacksize(&attr, stack_size);
    }

    // create
    auto thr = pthread_t{};
    const auto ret = ::pthread_create(&thr, &attr, func, data);
    ::pthread_attr_destroy(&attr);
    if (ret != 0) {
      return {};
    }

    return Thread{thr};
  }

  static auto yield_now() -> bool {
    const auto err = ::sched_yield();
    return err == 0;
  }

  static auto set_name(const char* name) -> bool {
#ifdef __APPLE__
    const auto err = ::pthread_setname_np(name);
#else
    const auto thr = ::pthread_self();
    const auto err = ::pthread_setname_np(thr, name);
#endif
    return err == 0;
  }

  auto is_valid() const -> bool {
    return _thr != 0;
  }

  auto join() -> bool {
    if (_thr == 0) return true;
    const auto err = ::pthread_join(_thr, nullptr);
    return err == 0;
  }

  auto detach() -> bool {
    if (_thr == 0) return true;
    const auto err = ::pthread_detach(_thr);
    return err == 0;
  }
};

inline auto sleep_ms(unsigned millis) -> bool {
  static constexpr auto MILLIS_PER_SEC = 1000U;
  static constexpr auto NANOS_PER_MILLI = 1000000U;

  auto ts = ::timespec{
      .tv_sec = millis / MILLIS_PER_SEC,
      .tv_nsec = (millis % MILLIS_PER_SEC) * NANOS_PER_MILLI,
  };

  const auto err = ::nanosleep(&ts, nullptr);
  return err == 0;
}

}  // namespace sfc::sys::unix
