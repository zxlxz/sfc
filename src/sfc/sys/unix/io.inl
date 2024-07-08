#pragma once

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sfc/core.h"

#undef stdin
#undef stdout
#undef stderr
#undef errno

namespace sfc::sys::io {

struct File {
  int _fd = -1;

 public:
  void close() {
    ::close(_fd);
  }

  auto read(auto buf) -> ssize_t {
    const auto ret = ::read(_fd, buf.as_mut_ptr(), buf.len());
    return ret;
  }

  auto write(auto buf) -> ssize_t {
    const auto ret = ::write(_fd, buf.as_ptr(), buf.len());
    return ret;
  }
};

struct Stdout {
  int _fd = STDOUT_FILENO;

 public:
  static auto instance() -> Stdout& {
    static auto res = Stdout{};
    return res;
  }

  auto is_tty() const -> bool {
    const auto res = ::isatty(_fd);
    return res == 1;
  }

  void write_str(auto buf) {
    ::fwrite(buf.as_ptr(), 1, buf.len(), ::stdout);
  }
};

static inline auto errno() -> int {
  return *__errno_location();
}

static inline auto error_str(int code) {
  return ::strerrorname_np(code);
}

}  // namespace sfc::sys::io
