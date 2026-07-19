#pragma once

#include "sfc/core.h"

namespace sfc::sys::windows {

class Mutex {
  struct Inn;
  Inn* _ptr{nullptr};

 public:
  explicit Mutex();
  ~Mutex();

  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&& other) noexcept;

 public:
  void* raw();
  void lock();
  void unlock();
  auto try_lock() -> bool;
};

class Condvar {
  struct Inn;
  Inn* _ptr{nullptr};

 public:
  Condvar();
  ~Condvar();

  Condvar(Condvar&& other) noexcept;
  Condvar& operator=(Condvar&& other) noexcept;

 public:
  void notify_one();
  void notify_all();

  void wait(Mutex& mtx);
  auto wait_timeout(Mutex& mtx, time::Duration dur) -> bool;
};

}  // namespace sfc::sys::windows
