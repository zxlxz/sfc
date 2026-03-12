#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

struct Thread {
  using ret_t = void*;
  using func_t = void* (*)(void*);
  pthread_t _thr = 0;

 public:
  static auto current() -> Thread {
    const auto thr = ::pthread_self();
    return Thread{thr};
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

  static auto yield() -> bool {
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

  auto tid() const -> unsigned {
    if (_thr == 0) return 0;
#ifdef __APPLE__
    auto thr = __uint64_t{0};
    ::pthread_threadid_np(_thr, &thr);
    return static_cast<unsigned>(thr);
#else
    const auto thr = ::syscall(SYS_gettid);
    return static_cast<unsigned>(thr);
#endif
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
