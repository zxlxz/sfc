#pragma once

// posix
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

// unix
#if defined(__unix__) || defined(__APPLE__)
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <pwd.h>
#include <sys/time.h>
#include <unistd.h>
#endif

// apple
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <_time.h>
#endif

namespace sfc::string {
class String;
}

namespace sfc::sys {

using string::String;

template <class S = String>
static auto make_string(const char* p) -> S {
  if (p == nullptr) {
    return {};
  }
  const auto n = __builtin_strlen(p);
  return S::from({p, n});
}

}  // namespace sfc::sys
