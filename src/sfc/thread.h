#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"

namespace sfc::thread {

#ifdef __unix__
using raw_t = u64;
#else
using raw_t = void*;
#endif

struct Thread {
  raw_t _raw{};

 public:
  static auto current() -> Thread;

  auto name() const -> String;

  void join();
};

class JoinHandle {
  friend struct Builder;
  Thread _thrd{};

 public:
  explicit JoinHandle(Thread thrd) noexcept;
  ~JoinHandle();

  JoinHandle(JoinHandle&&) noexcept;
  auto operator=(JoinHandle&&) noexcept -> JoinHandle&;
};

struct Builder {
  usize stack_size = 0;
  Str name = {};

 public:
  auto spawn(Box<void()> f) const -> JoinHandle;
};

void sleep(const time::Duration& dur);

auto spawn(auto f) -> JoinHandle {
  return Builder{}.spawn(Box<void()>::xnew(mem::move(f)));
}

}  // namespace sfc::thread
