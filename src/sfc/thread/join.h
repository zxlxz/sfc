#pragma once

#include "sfc/alloc.h"
#include "sfc/thread/thread.h"

namespace sfc::thread {

class JoinHandle {
  friend struct Builder;
  sys::Thread _thread{};

 public:
  JoinHandle() noexcept;
  ~JoinHandle() noexcept;

  JoinHandle(JoinHandle&& other) noexcept;
  JoinHandle& operator=(JoinHandle&& other) noexcept;

  auto is_finished() const -> bool;
  void join();
};

struct Builder {
  usize stack_size = 0;
  Str name = {};

 public:
  auto spawn(Box<void()> f) -> JoinHandle;
};

auto spawn(auto f) -> JoinHandle {
  return Builder{}.spawn(Box<void()>::xnew(mem::move(f)));
}

}  // namespace sfc::thread
