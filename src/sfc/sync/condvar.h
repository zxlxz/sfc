#pragma once

#include "sfc/sync/mutex.h"
#include "sfc/time.h"

namespace sfc::sync {

class Condvar {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit Condvar() noexcept;
  ~Condvar() noexcept;

  Condvar(Condvar&&) noexcept;
  Condvar& operator=(Condvar&&) noexcept;

  void notify_one() noexcept;
  void notify_all() noexcept;

  auto wait(Mutex::Guard& lock) noexcept -> bool;
  auto wait_timeout(Mutex::Guard& lock, time::Duration dur) noexcept -> bool;

  void wait_while(Mutex::Guard& lock, auto&& cond) {
    while (auto x = cond()) {
      this->wait(lock);
    }
  }
};

}  // namespace sfc::sync
