#pragma once

#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "sfc/alloc.h"

namespace sfc::sys::thread {

struct Thread {
  using tid_t = pthread_t;

  tid_t _raw;

public:

  static auto current() -> Thread {
    return Thread{::pthread_self()};
  }

  static auto start(usize stack_size, void* (*func)(void*), void* data) -> Thread {
    // attr
    auto attr = ::pthread_attr_t{};
    ::pthread_attr_init(&attr);
    if (stack_size != 0) {
      ::pthread_attr_setstacksize(&attr, stack_size);
    }

    // create
    auto thrd = pthread_t{0};
    (void)::pthread_create(&thrd, &attr, func, data);
    ::pthread_attr_destroy(&attr);

    return Thread{thrd};
  }

  operator bool() const {
    return _raw != 0;
  }

  auto id() const-> int {
    return ::gettid();
  }

  auto join() -> bool {
    const auto err = ::pthread_join(_raw, nullptr);
    return err == 0;
  }

  auto detach() -> bool {
    const auto err = ::pthread_detach(_raw);
    return err == 0;
  }

  auto get_name(char* buf, size_t buf_len) const -> size_t {
    const auto err = ::pthread_getname_np(_raw, buf, sizeof(buf));
    if (err != 0) {
      return 0;
    }

    const auto len = ::strnlen(buf, buf_len);
    return len;
  }

  auto set_name(const char* name) -> bool {
    if (name == nullptr) {
      return false;
    }

    const auto err = ::pthread_setname_np(_raw, name);
    return err == 0;
  }
};

static inline auto sleep_ms(time_t millis) -> bool {
  using timespec_t = struct ::timespec;

  const auto req = timespec_t{
      .tv_sec = millis / 1000,
      .tv_nsec = millis % 1000 * 1000000,
  };

  const auto ret = ::nanosleep(&req, nullptr);
  return ret != -1;
}

}  // namespace sfc::sys::thread
