#pragma once

#include "sfc/alloc.h"

namespace sfc::sync {

class Mutex {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit Mutex() noexcept;
  ~Mutex() noexcept;

  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&&) noexcept;

 public:
  struct Guard {
    Inn* _inn;

   public:
    explicit Guard(Inn* mtx) noexcept;
    ~Guard() noexcept;

    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
  };

  auto lock() noexcept -> Guard;
  auto try_lock() noexcept -> Option<Guard>;
};

class ReentrantLock {
  struct Inn;
  Box<Inn> _inn;

 public:
  explicit ReentrantLock() noexcept;
  ~ReentrantLock() noexcept;

  ReentrantLock(ReentrantLock&&) noexcept;
  ReentrantLock& operator=(ReentrantLock&&) noexcept;

 public:
  struct Guard {
    Inn* _inn;

   public:
    explicit Guard(Inn* mtx) noexcept;
    ~Guard() noexcept;

    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
  };

  auto lock() noexcept -> Guard;
  auto try_lock() noexcept -> Option<Guard>;
};

}  // namespace sfc::sync
