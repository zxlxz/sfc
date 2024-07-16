#pragma once

#include <errno.h>
#include <pthread.h>

#include "sfc/alloc.h"

namespace sfc::sys::thread {

struct Thread {
  pthread_t _id;

 public:
  static auto xnew(usize stack_size, Box<void()> fun) -> Option<Thread> {
    // attr
    auto attr = ::pthread_attr_t{};
    ::pthread_attr_init(&attr);
    if (stack_size != 0) {
      ::pthread_attr_setstacksize(&attr, stack_size);
    }

    // create
    auto tid = pthread_t{0};
    const auto ptr = Box<Box<void()>>::xnew(mem::move(fun)).into_raw();
    const auto err = ::pthread_create(&tid, &attr, &Thread::start_routine, ptr);

    ::pthread_attr_destroy(&attr);

    if (err != 0) {  // err check
      (void)Box<Box<void()>>::from_raw(ptr);
      return {};
    }

    return Thread{tid};
  }

  static auto current() -> Thread {
    const auto tid = ::pthread_self();
    return Thread{tid};
  }

  auto id() const -> pthread_t {
    return _id;
  }

  auto name() const -> String {
    char buf[64] = "";
    const auto err = ::pthread_getname_np(_id, buf, sizeof(buf));
    if (err != 0) {  // err check
      return {};
    }
    auto res = String::from(buf);
    return res;
  }

  auto set_name(cstr_t name) -> bool {
    if (name == nullptr) {
      return false;
    }
    const auto err = ::pthread_setname_np(_id, name);
    return err == 0;
  }

  auto join() -> bool {
    const auto err = ::pthread_join(_id, nullptr);
    return err == 0;
  }

  auto detach() -> bool {
    const auto err = ::pthread_detach(_id);
    return err == 0;
  }

 private:
  static auto start_routine(void* raw) -> void* {
    auto fun = Box<Box<void()>>::from_raw(static_cast<Box<void()>*>(raw));
    (*fun)();
    return nullptr;
  }
};

static inline auto sleep(auto dur) -> bool {
  struct timespec ts = {
      .tv_sec = static_cast<time_t>(dur.as_secs()),
      .tv_nsec = static_cast<time_t>(dur.subsec_nanos()),
  };

  const auto ret = ::nanosleep(&ts, &ts);
  if (ret == -1) {
    return false;
  }
  return true;
}

}  // namespace sfc::sys::thread
