#pragma once

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sfc/io/mod.h"

#undef stdin
#undef stdout
#undef stderr
#undef errno

namespace sfc::sys::io {

using sfc::io::ErrorKind;

static constexpr fd_t INVALID_FD = -1;

struct File {
  int _fd = -1;

 public:
  operator bool() const {
    return _fd != -1;
  }

  void close() {
    ::close(_fd);
  }

  auto read(Slice<u8> buf) -> ssize_t {
    const auto ptr = buf.as_mut_ptr();
    const auto len = buf.len();
    const auto ret = ::read(_fd, ptr, len);
    return ret;
  }

  auto write(Slice<const u8> buf) -> ssize_t {
    const auto ptr = buf.as_ptr();
    const auto len = buf.len();
    const auto ret = ::write(_fd, ptr, len);
    return ret;
  }
};

struct Stdout {
  static auto is_tty() -> bool {
    const auto res = ::isatty(STDOUT_FILENO);
    return res == 1;
  }

  static void flush() {
    ::fflush(::stdout);
  }

  static void write_str(auto buf) {
    ::fwrite(buf.as_ptr(), 1, buf.len(), ::stdout);
  }
};

static inline auto last_err() -> int {
  return *__errno_location();
}

static inline auto error_str(int code) {
  return ::strerrorname_np(code);
}

static inline auto err_kind(int code) -> ErrorKind {
  // clang-format off
  switch(code) {
    case EACCES:        return ErrorKind::PermissionDenied;
    case EPERM:         return ErrorKind::PermissionDenied;
    case EADDRINUSE:    return ErrorKind::AddrInUse;
    case EADDRNOTAVAIL: return ErrorKind::AddrNotAvailable;
    case ECONNABORTED:  return ErrorKind::ConnectionAborted;
    case ECONNREFUSED:  return ErrorKind::ConnectionRefused;
    case ECONNRESET:    return ErrorKind::ConnectionReset;
    case EEXIST:        return ErrorKind::AlreadyExists;
    case EINTR:         return ErrorKind::Interrupted;
    case EINVAL:        return ErrorKind::InvalidInput;
    case ENOENT:        return ErrorKind::NotFound;
    case ENOTCONN:      return ErrorKind::NotConnected;
    case EPIPE:         return ErrorKind::BrokenPipe;
    case ETIMEDOUT:     return ErrorKind::TimedOut;
    case EWOULDBLOCK:   return ErrorKind::WouldBlock;
    default:            return ErrorKind::Other;
  }
  // clang-format on
}

}  // namespace sfc::sys::io
