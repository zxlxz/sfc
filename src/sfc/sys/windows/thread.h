#pragma once

#include "sfc/alloc.h"

namespace sfc::sys::windows {

struct Thread {
  void* _handle{nullptr};

 public:
  static auto spawn(usize stack_size, Box<void()> f) -> Thread;

 public:
  void join();
  void detach();
};

namespace this_thread {
auto id() noexcept -> u32;
auto set_name(const char* name) -> bool;

void yield();
void sleep(time::Duration dur) noexcept;
};  // namespace this_thread

}  // namespace sfc::sys::windows
