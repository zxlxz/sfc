#pragma once

#include "sfc/sys.h"

namespace sfc::thread {

struct Thread {
  u32 id;
};

auto current() -> Thread;
auto current_id() -> u32;

void yield_now();

void sleep(time::Duration dur);
void sleep_ms(u32 ms);

}  // namespace sfc::thread
