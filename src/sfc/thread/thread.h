#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"

namespace sfc::thread {

struct Thread {
#ifdef __unix__
  u64 _raw = 0;
#else
  void* _raw = nullptr;
#endif

 public:
  void join();
};

class JoinHandle {
  friend struct Builder;
  Thread _thrd{};

 public:
  JoinHandle() noexcept = default;

  ~JoinHandle() noexcept {
    if (_thrd._raw) {
      _thrd.join();
    }
  }

  JoinHandle(JoinHandle&& other) noexcept : _thrd{mem::take(other._thrd)} {}

  JoinHandle& operator=(JoinHandle&& other) noexcept {
    if (this != &other) {
      mem::swap(_thrd, other._thrd);
    }
    return *this;
  }
};

struct Builder {
  usize stack_size = 0;
  Str name = {};

 public:
  auto spawn(Box<void()> f) -> JoinHandle;
};

auto current() -> Thread;

void yield_now();

void sleep(time::Duration dur);

void sleep_ms(u32 ms);

auto spawn(auto f) -> JoinHandle {
  return Builder{}.spawn(Box<void()>::xnew(mem::move(f)));
}

}  // namespace sfc::thread
