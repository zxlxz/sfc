#pragma once

#include "sfc/alloc.h"

namespace sfc::sys::windows {

using ThreadHandle = void*;

struct Thread {
  ThreadHandle _handle{nullptr};

 public:
  static auto spawn(usize stack_size, Box<void()> f) -> Thread;

 public:
  void join();
  void detach();
};

struct ThisThread {
  static auto id() noexcept -> u32;
  static auto set_name(const char* name) -> bool;

  static void yield();
  static void sleep(time::Duration dur) noexcept;
};

}  // namespace sfc::sys::windows
