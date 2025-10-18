#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"

namespace sfc::thread {

struct Thread {
#ifdef __unix__
  u64 _raw{};
#else
  void* _raw{};
#endif
};

class JoinHandle {
  Thread _thrd{};

 public:
  explicit JoinHandle(Thread thrd) noexcept;
  ~JoinHandle() noexcept;

  JoinHandle(JoinHandle&&) = delete;
  JoinHandle& operator=(JoinHandle&&) = delete;

  JoinHandle(const JoinHandle&) = delete;
  JoinHandle& operator=(const JoinHandle&) = delete;
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
