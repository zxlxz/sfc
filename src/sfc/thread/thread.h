#pragma once

#include "sfc/alloc.h"

namespace sfc::thread {

#if defined(_WIN32)
using ThreadHandle = void*;
#elif defined(__APPLE__)
using ThreadHandle = struct _opaque_pthread_t*;
#elif defined(__linux__)
using ThreadHandle = unsigned long;
#endif

struct SysThread {
  ThreadHandle _raw{0};

 public:
  void join();
};

struct Thread {
  u32 id;
};

auto current() -> Thread;
auto current_id() -> u32;

void yield_now();

void sleep(time::Duration dur);
void sleep_ms(u32 ms);

}  // namespace sfc::thread
