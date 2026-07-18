#pragma once

#include "sfc/core/time.h"

namespace sfc::sys::posix {

class Mutex {
  struct Inn;
  Inn* _ptr{nullptr};

 public:
  explicit Mutex();
  ~Mutex();

  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&& other) noexcept;

 public:
  auto inn() -> Inn&;
  void lock();
  void unlock();
  auto try_lock() -> bool;
};

class Condvar {
  struct Inn;
  Inn* _ptr;

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

}  // namespace sfc::sys::posix
