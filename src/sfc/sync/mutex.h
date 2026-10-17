#pragma once

#include "sfc/core.h"

namespace sfc::sync {

class Mutex {
  struct Inn;
  Inn* _inn{nullptr};

 public:
  explicit Mutex() noexcept;
  ~Mutex() noexcept;
  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&&) noexcept;

 public:
  class Guard;
  auto lock() const noexcept -> Guard;
  auto try_lock() const noexcept -> Option<Guard>;
};

class ReentrantLock {
  struct Inn;
  Inn* _inn{nullptr};

 public:
  explicit ReentrantLock() noexcept;
  ~ReentrantLock() noexcept;
  ReentrantLock(ReentrantLock&&) noexcept;
  ReentrantLock& operator=(ReentrantLock&&) noexcept;

 public:
  class Guard;
  auto lock() const noexcept -> Guard;
  auto try_lock() const noexcept -> Option<Guard>;
};

class Mutex::Guard {
  Inn* _inn{nullptr};

 public:
  Guard(Inn& mtx) noexcept;
  Guard(Guard&&) noexcept;
  ~Guard() noexcept;
  Guard& operator=(Guard&&) noexcept = delete;

 public:
  auto inner() const -> Inn&;
};

class ReentrantLock::Guard {
  Inn* _inn{nullptr};

 public:
  Guard(Inn& inn) noexcept;
  Guard(Guard&&) noexcept;
  ~Guard() noexcept;
  Guard& operator=(Guard&&) noexcept = delete;
};

}  // namespace sfc::sync
