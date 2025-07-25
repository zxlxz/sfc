#pragma once

#include "sfc/sync/mutex.h"

namespace sfc::sync {

class Condvar {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit Condvar();
  ~Condvar();

  Condvar(Condvar&&) noexcept;
  Condvar& operator=(Condvar&&) noexcept;

  void notify_one();
  void notify_all();

  void wait(Mutex::Guard& lock);
  auto wait_timeout(Mutex::Guard& lock, const time::Duration& dur) -> bool;
};

}  // namespace sfc::sync
