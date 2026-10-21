#pragma once

#include "sfc/alloc.h"

struct _opaque_pthread_t;

namespace sfc::sys::posix {

struct Thread {
#ifdef __APPLE__
  struct _opaque_pthread_t* _handle;
#else
  unsigned long _handle;
#endif

 public:
  static auto new_(usize stack_size, Box<void()> f) -> Thread;
  void join();
  void detach();
};

namespace this_thread {
auto id() noexcept -> u32;
auto set_name(const char* name) -> bool;

void yield();
void sleep(time::Duration dur) noexcept;
};  // namespace this_thread

}  // namespace sfc::sys::posix
