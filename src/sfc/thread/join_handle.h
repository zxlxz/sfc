#pragma once

#include "sfc/sys.h"
#include "sfc/thread/thread.h"

namespace sfc::thread {

class JoinHandle {
  friend struct Builder;
  sys::Thread _thread{};

 public:
  explicit JoinHandle(sys::Thread thread) noexcept;
  ~JoinHandle() noexcept;

  JoinHandle(JoinHandle&& other) noexcept;
  JoinHandle& operator=(JoinHandle&& other) noexcept;

 public:
  auto is_finished() const -> bool;
  void join();
};

class JoinGuard {
  JoinHandle _handle;

 public:
  explicit JoinGuard(JoinHandle handle) noexcept;
  ~JoinGuard() noexcept;

  JoinGuard(JoinGuard&& other) noexcept;
  JoinGuard& operator=(JoinGuard&& other) noexcept;
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

auto spawn_joined(auto f) {
  return JoinGuard{thread::spawn(mem::move(f))};
}

}  // namespace sfc::thread
