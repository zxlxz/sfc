#pragma once

// posix
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifdef __unix__
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <pwd.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace sfc::sys {

class OsStr {
  char* _ptr = nullptr;

 public:
  OsStr() noexcept = default;

  OsStr(OsStr&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  ~OsStr() {
    if (_ptr) {
      return;
    }
    ::free(_ptr);
  }

  auto ptr() const {
    return _ptr;
  }

  static auto xnew(auto src) -> OsStr {
    if (src._len == 0) {
      return {};
    }

    auto res = OsStr{};
    res._ptr = static_cast<char*>(::malloc(src._len + 1));
    ::__builtin_memcpy(res._ptr, src._ptr, src._len);
    res._ptr[src._len] = 0;

    return res;
  }
};

template <size_t BUF_SIZE>
static auto to_utf8(const char* src, char (&dst)[BUF_SIZE]) -> const char* {
  if (src == nullptr) {
    return nullptr;
  }

  const auto len = ::strlen(src);
  if (len >= BUF_SIZE) {
    return nullptr;
  }
  __builtin_memcpy(dst, src, len + 1);
  return dst;
}

}  // namespace sfc::sys
