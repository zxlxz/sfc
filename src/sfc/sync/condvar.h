#pragma once

#include "sfc/sync/mutex.h"

namespace sfc::sync {

class Condvar {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit Condvar();
  ~Condvar()noexcept;

  Condvar(Condvar&&) noexcept;
  Condvar& operator=(Condvar&&) noexcept;

  void notify_one();
  void notify_all();

  auto wait(Mutex::Guard& lock) -> bool;
  auto wait_timeout(Mutex::Guard& lock, time::Duration dur) -> bool;

  void wait_while(Mutex::Guard& lock, auto&& cond) {
    while (auto x = cond()) {
      this->wait(lock);
    }
  }
};

}  // namespace sfc::sync
