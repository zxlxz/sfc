#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"

namespace sfc::thread {

#ifdef _WIN32
using thrd_t = void*;
#else
using thrd_t = uint64_t;
#endif

struct Thread {
  thrd_t _raw = thrd_t{0};

 public:
  static auto current() -> Thread;

  auto name() const -> String;
};

struct Builder {
  usize stack_size = 0;
  String name      = {};

 public:
  auto spawn(Box<void()> f) -> class JoinHandle;
};

class JoinHandle {
  friend struct Builder;
  Thread _thr{};

 public:
  explicit JoinHandle() noexcept;
  JoinHandle(JoinHandle&&) noexcept;
  ~JoinHandle();

  auto operator=(JoinHandle&&) noexcept -> JoinHandle&;
  void join();
};

void sleep(const time::Duration& dur);

auto spawn(auto f) -> JoinHandle {
  return Builder{}.spawn(Box<void()>::xnew(mem::move(f)));
}

}  // namespace sfc::thread
