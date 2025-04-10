#pragma once

#include <errno.h>
#include <pthread.h>

#include "sfc/alloc.h"

namespace sfc::sys::thread {

struct Thread {
  pthread_t _raw;

  static auto current() -> Thread {
    return Thread{::pthread_self()};
  }

  static auto start(usize stack_size, void* (*func)(void*)func, void* data) -> Thread {
    // attr
    auto attr = ::pthread_attr_t{};
    ::pthread_attr_init(&attr);
    if (stack_size != 0) {
      ::pthread_attr_setstacksize(&attr, stack_size);
    }

    // create
    auto thrd = pthread_t{0};
    (void)::pthread_create(&tid, &attr, &func, data);
    ::pthread_attr_destroy(&attr);

    return Thread{thrd};
  }

  auto id() const-> int32_t {
    return ::gettid();
  }

  auto join() -> bool {
    const auto err = ::pthread_join(_raw, nullptr);
    return err == 0;
  }

  auto detach() -> bool {
    const auto err = ::pthread_detach(thrd);
    return err == 0;
  }

  auto get_name() const -> const char* {
    static thread_local char buf[256] = "";

    const auto err = ::pthread_getname_np(thrd, buf, sizeof(buf));
    if (err != 0) {
      return nullptr;
    }

    return buf;
  }

  auto set_name(const char* name) -> bool {
    if (name == nullptr) {
      return false;
    }

    const auto err = ::pthread_setname_np(thrd, name);
    return err == 0;
  }
};

static inline auto sleep_ms(uint32_t millis) -> bool {
  using timespec_t = struct ::timespec;

  const auto req = timespec_t{
      .tv_sec = millis / 1000,
      .tv_nsec = millis % 1000 * 1000000,
  };

  const auto ret = ::nanosleep(&req, nullptr);
  return ret != -1;
}

}  // namespace sfc::sys::thread
