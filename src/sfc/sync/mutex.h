#pragma once

#include "sfc/alloc.h"

namespace sfc::sync {

class Mutex {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit Mutex();
  ~Mutex() noexcept;

  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&&) noexcept;

  class Guard;
  auto lock() -> Guard;
};

class [[nodiscard]] Mutex::Guard {
  friend class Condvar;
  Inn& _inn;

 public:
  explicit Guard(Inn&) noexcept;
  ~Guard() noexcept;

  Guard(const Guard&) = delete;
  Guard& operator=(const Guard&) = delete;
};

class ReentrantLock {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit ReentrantLock() noexcept;
  ~ReentrantLock() noexcept;

  ReentrantLock(ReentrantLock&&) noexcept;
  ReentrantLock& operator=(ReentrantLock&&) noexcept;

  class Guard;
  auto lock() -> Guard;
};

class [[nodiscard]] ReentrantLock::Guard {
  Inn& _inn;

 public:
  explicit Guard(Inn& mtx) noexcept;
  ~Guard() noexcept;

  Guard(const Guard&) = delete;
  Guard& operator=(const Guard&) = delete;
};

}  // namespace sfc::sync
