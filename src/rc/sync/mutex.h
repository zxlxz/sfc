#pragma once

#include "rc/sys.h"

namespace rc::sync {

struct Mutex {
  sys::sync::Mutex _inner;

  pub Mutex();
  pub ~Mutex();
  pub Mutex(Mutex&&) noexcept;

  struct Guard;
  pub [[nodiscard]] auto lock() -> Guard;
};

struct Mutex::Guard {
  Mutex* _mutex;
  pub explicit Guard(Mutex& mtx) noexcept;
  pub ~Guard();
  pub Guard(Guard&& other) noexcept;
};

using MutexGuard = Mutex::Guard;

}  // namespace rc::sync
