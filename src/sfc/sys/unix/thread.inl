#pragma once

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "sfc/alloc.h"

namespace sfc::sys::thread {

using timespec_t   = struct ::timespec;
using timeval_t    = struct ::timeval;
using thread_ret_t = void*;

struct Thread {
  pthread_t _raw{0};

 public:
  static auto current() -> Thread {
    return Thread{::pthread_self()};
  }

  static auto start(pthread_t stack_size, void* (*func)(void*), void* data) -> Thread {
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

  static auto from_raw(pthread_t raw) -> Thread {
    return Thread{raw};
  }

  auto raw() const -> pthread_t {
    return _raw;
  }

  operator bool() const {
    return _raw != 0;
  }

  auto id() const -> int {
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

struct Mutex {
  pthread_mutex_t _raw = PTHREAD_MUTEX_INITIALIZER;

 public:
  void lock() {
    ::pthread_mutex_lock(&_raw);
  }

  void unlock() {
    ::pthread_mutex_unlock(&_raw);
  }
};

struct Condvar {
  pthread_cond_t _raw = PTHREAD_COND_INITIALIZER;

  void notify_one() {
    ::pthread_cond_signal(&_raw);
  }

  void notify_all() {
    ::pthread_cond_broadcast(&_raw);
  }

  void wait(Mutex& mtx) {
    ::pthread_cond_wait(&_raw, &mtx._raw);
  }

  auto wait_timeout_ms(Mutex& mtx, unsigned millis) -> bool {
    auto time_now = timeval_t{};
    ::gettimeofday(&time_now, nullptr);

    auto time_out    = timespec_t{};
    time_out.tv_sec  = time_now.tv_sec + millis / 1000;
    time_out.tv_nsec = time_now.tv_usec * 1000 + (millis % 1000) * 1000;

    if (time_out.tv_nsec >= 1000000000) {
      time_out.tv_sec += 1;
      time_out.tv_nsec -= 1000000000;
    }

    const auto err = ::pthread_cond_timedwait(&_raw, &mtx._raw, &time_out);
    return err == 0;
  }
};

static inline auto sleep_ms(unsigned millis) -> bool {
  const auto req = timespec_t{
      .tv_sec  = millis / 1000,
      .tv_nsec = millis % 1000 * 1000000,
  };

  const auto ret = ::nanosleep(&req, nullptr);
  return ret != -1;
}

}  // namespace sfc::sys::thread
