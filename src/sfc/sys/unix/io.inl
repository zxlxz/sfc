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

struct Error {
  int _code = 0;

 public:
  static auto last() -> Error {
    return {*__errno_location()};
  }

  auto code() const-> int {
    return _code;
  }

  auto kind() const -> sfc::io::ErrorKind {
    // clang-format off
    switch(_code) {
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

};

struct File {
  int _fd = -1;

 public:
  explicit operator bool() const {
    return _fd != -1;
  }

  void close() {
    if (_fd == -1) {
      return;
    }
    ::close(_fd);
  }

  void flush() {
    if (_fd != -1) {
      return;
    }
    ::fsync(_fd);
  }

  auto read(void* buf, size_t buf_size) -> ssize_t {
    if (buf == nullptr || buf_size == 0) {
      return 0;
    }
    if (_fd == -1) {
      return -1;
    }

    const auto ret = ::read(_fd, buf, buf_size);
    return ret;
  }

  auto write(const void* buf, size_t buf_size) -> ssize_t {
    if (buf == nullptr || buf_size == 0) {
      return 0;
    }
    if (_fd == -1) {
      return -1;
    }
    const auto ret = ::write(_fd, buf, buf_size);
    return ret;
  }

};

}  // namespace sfc::sys::io
