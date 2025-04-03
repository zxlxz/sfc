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
  auto wait_timeout(LockGuard& lock, const time::Duration& dur) -> bool;
};

}  // namespace sfc::sync
