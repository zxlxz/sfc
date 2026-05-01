#pragma once

#include "sfc/sys/unix/mod.inl"
#define _SFC_SYS_THREAD_

namespace sfc::sys::unix {

struct ThreadAttr {
  pthread_attr_t _raw;

  ThreadAttr() {
    ::pthread_attr_init(&_raw);
  }

  ~ThreadAttr() {
    ::pthread_attr_destroy(&_raw);
  }

  void set_stack_size(size_t stack_size) {
    if (stack_size == 0) return;
    ::pthread_attr_setstacksize(&_raw, stack_size);
  }
};

struct Thread {
  using func_t = void* (*)(void*);
  pthread_t _raw = {};

 public:
  static auto current() -> u32 {
#ifdef __APPLE__
    auto tid = ino_t{0};
    ::pthread_threadid_np(nullptr, &tid);
#else
    static thread_local const auto tid = ::syscall(SYS_gettid);
#endif
    return static_cast<u32>(tid);
  }

  template <class Fn>
  static auto callback(void* p) -> void* {
    const auto ret = Fn::run(p);
    return ret ? nullptr : reinterpret_cast<void*>(-1LL);
  }

  template <class Fn>
  static auto spawn(size_t stack_size, Fn* func) -> Thread {
    // thread attr
    auto attr = ThreadAttr{};
    attr.set_stack_size(stack_size);

    // create
    auto thr = pthread_t{};
    if (auto err = ::pthread_create(&thr, &attr._raw, callback<Fn>, func); err != 0) {
      return {};
    }

    return Thread{thr};
  }

  static void yield_now() {
    (void)::sched_yield();
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
    return _raw != 0;
  }

  void join() {
    sfc::expect(this->is_valid(), "Thread::join: invalid thread");
    // just join, don't change thrd, the caller should handle it
    const auto err = ::pthread_join(_raw, nullptr);
    sfc::expect(err == 0, "Thread::join: pthread_join failed, err={}", err);
  }

  void detach() {
    sfc::expect(this->is_valid(), "Thread::detach: invalid thread");
    // just detach, don't change thrd, the caller should handle it
    const auto err = ::pthread_detach(_raw);
    sfc::expect(err == 0, "Thread::detach: pthread_detach failed, err={}", err);
  }
};

inline void sleep(time::Duration dur) noexcept {
  using namespace time;

  if (dur._nanos <= NANOS_PER_MILLI) {
    ::sched_yield();
    return;
  }

  const auto ts = ::timespec{
      .tv_sec = static_cast<time_t>(dur.as_secs()),
      .tv_nsec = dur.subsec_nanos(),
  };

  (void)::nanosleep(&ts, nullptr);
}

}  // namespace sfc::sys::unix
