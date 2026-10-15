#pragma once

#include "sfc/core.h"
#include "sfc/sys/sync.h"

namespace sfc::sync {

struct Tid {
  Atomic<u32> _0{0};

 public:
  auto contains(u32 owner) const -> bool;
  void set(u32 owner);
};

class Mutex {
  struct Inn : sys::Mutex {};
  mutable Inn _inn;

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
  struct Inn : sys::Mutex {
    Tid _owner{0};
    u32 _count{0};
  };
  mutable Inn _inn;

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
  auto inner() const -> sys::Mutex&;
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
