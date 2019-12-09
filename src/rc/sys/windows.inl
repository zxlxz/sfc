#pragma once

#if __has_include(<Windows.h>)
#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>
#endif

#include "rc.inl"

#define _rc_SYS_WINDOWS_IMPL_
#include "rc/sys/windows.h"

namespace rc::sys::windows {

using namespace rc::time;

inline auto dur2timeout(Duration dur) -> DWORD {
  const auto r = dur._secs * MILLIS_PER_SEC + dur._nanos / NANOS_PER_MILLIS;
  if (r >= u64(INFINITE)) {
    return INFINITE;
  }
  return u32(r);
}

}  // namespace rc::sys::windows
