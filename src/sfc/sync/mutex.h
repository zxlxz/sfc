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

class ReentrantLock {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit ReentrantLock();
  ~ReentrantLock();

  ReentrantLock(ReentrantLock&&) noexcept = default;
  ReentrantLock& operator=(ReentrantLock&&) noexcept = default;

  class Guard;
  [[nodiscard]] auto lock() -> Guard;
};

class [[nodiscard]] ReentrantLock::Guard {
  ptr::Unique<Inn> _mtx = {};

 public:
  explicit Guard(Inn& mtx);
  ~Guard() noexcept;

  Guard(Guard&&) noexcept = default;
  Guard& operator=(Guard&&) noexcept = default;
};

}  // namespace sfc::sync
