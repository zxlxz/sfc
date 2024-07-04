#pragma once

#include "mutex.h"

namespace sfc::sync {

class Condvar {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit Condvar();
  ~Condvar();
  Condvar(Condvar&&) noexcept;

  void notify_one();
  void notify_all();

  void wait(LockGuard& lock);
  auto wait_timeout(LockGuard& lock, time::Duration dur) -> bool;
  auto wait_timeout_ms(LockGuard& lock, u32 ms) -> bool;
};

}  // namespace sfc::sync
