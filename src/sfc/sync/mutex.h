#pragma once

#include "sfc/alloc.h"
#include "sfc/time.h"

namespace sfc::sync {

class Mutex {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit Mutex();
  ~Mutex();
  Mutex(Mutex&&) noexcept;
  Mutex& operator=(Mutex&&) noexcept;

  class Guard;
  auto lock() -> Guard;
};

class [[nodiscard]] Mutex::Guard {
  friend class Condvar;
  ptr::Unique<Inn> _mtx = {};

 public:
  explicit Guard(Inn& mtx);
  ~Guard();
};

}  // namespace sfc::sync
