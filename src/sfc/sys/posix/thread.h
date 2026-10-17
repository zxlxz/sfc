#pragma once

#include "sfc/alloc.h"

struct _opaque_pthread_t;

namespace sfc::sys::posix {

#ifdef __APPLE__
using ThreadHandle = struct _opaque_pthread_t*;
#else
using ThreadHandle = unsigned long;
#endif

struct Thread {
  ThreadHandle _handle{};

 public:
  static auto spawn(usize stack_size, Box<void()> f) -> Thread;
  void join();
  void detach();
};

struct ThisThread {
  static auto id() noexcept -> u32;
  static auto set_name(const char* name) -> bool;

  static void yield();
  static void sleep(time::Duration dur) noexcept;
};

}  // namespace sfc::sys::posix
