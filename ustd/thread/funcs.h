#pragma once

#include <ustd/time.h>

namespace ustd::thread
{
fn sleep(time::Duration dur) -> void;
fn sleep_ms(u32 ms)          -> void;
}
