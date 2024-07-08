#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"

namespace sfc::thread {

struct Thread {
  static constexpr u64 INVALID_ID = static_cast<u64>(-1);
  u64 _id = INVALID_ID;

 public:
  static auto current() -> Thread;

  auto id() const -> u64;

  auto name() const -> String;
};

struct Builder {
  usize _stack_size = 0;
  String _name = {};

 public:
  Builder() = default;

  auto spawn(Box<void()> f) -> class JoinHandle;
};

class JoinHandle {
  Thread _thr;

 public:
  JoinHandle();
  explicit JoinHandle(Thread thr);
  JoinHandle(JoinHandle&&) noexcept;
  ~JoinHandle();

  auto operator=(JoinHandle&&) noexcept -> JoinHandle&;

  auto thread() const -> Thread;

  void join();
};

void sleep(time::Duration dur);

auto spawn(auto f) -> JoinHandle {
  return Builder{}.spawn(Box<void()>{f});
}

}  // namespace sfc::thread
