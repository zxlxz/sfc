#pragma once

#include "sfc/core.h"

// raw system types, not intended to be used directly
// it only provides the raw types for memory layout compatibility
// the actual implementation is in platform-specific modules
namespace sfc::sys::raw {

#ifdef _WIN32
using RawFd = void*;
#else
using RawFd = int;
#endif

#ifdef _WIN32
using ErrCode = u32;
#else
using ErrCode = int;
#endif

struct File {
  RawFd _raw;
};

struct Thread {
  void* _raw;
};

struct Mutex {
  void* _raw;
};

struct Condvar {
  void* _raw;
};

}  // namespace sfc::sys::raw
