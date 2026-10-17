#pragma once

#include "sfc/alloc.h"
#include "sfc/thread/thread.h"

namespace sfc::thread {

class JoinHandle {
  friend struct Builder;
  SysThread _thrd{0};

 public:
  explicit JoinHandle(SysThread thrd) noexcept;
  ~JoinHandle() noexcept;

  JoinHandle(JoinHandle&& other) noexcept;
  JoinHandle& operator=(JoinHandle&& other) noexcept;

 public:
  auto is_finished() const -> bool;
  void join();
};

struct Builder {
  usize stack_size = 0;

 public:
  auto spawn(Box<void()> f) -> JoinHandle;
};

auto spawn(auto f) -> JoinHandle {
  auto b = Builder{};
  return b.spawn(Box<void()>::new_(mem::move(f)));
}

}  // namespace sfc::thread
