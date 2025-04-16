#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"

namespace sfc::thread {

#ifdef _WIN32
using thrd_t = void*;
#else
using thrd_t = u64;
#endif

struct Thread {
  thrd_t _raw = thrd_t{0};

 public:
  static auto current() -> Thread;

  auto id() const -> i64;

  auto name() const -> String;
};

class JoinHandle {
  friend struct Builder;
  Thread _thrd{};

 public:
  explicit JoinHandle() noexcept;
  ~JoinHandle();

  JoinHandle(JoinHandle&&) noexcept;
  JoinHandle& operator=(JoinHandle&&) noexcept;

  void join();
};

struct Builder {
  usize stack_size = 0;
  Str   name = {};

 public:
  auto spawn(Box<void()> f) const -> JoinHandle;
};

void sleep(const time::Duration& dur);

auto spawn(auto f) -> JoinHandle {
  return Builder{}.spawn(Box<void()>::xnew(mem::move(f)));
}

}  // namespace sfc::thread
