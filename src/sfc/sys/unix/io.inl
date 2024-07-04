#pragma once
#if defined(__unix__) || defined(__APPLE__)

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#undef stdin
#undef stdout
#undef stderr

namespace sfc::sys::io {

using stat_t = struct stat;

static constexpr auto INVALID_FD = -1;

static inline auto get_err() -> int {
  return errno;
}

template <class T>
static inline auto kind_of(int code) -> T {
  switch (code) {
    case EACCES:        return T::PermissionDenied;
    case EPERM:         return T::PermissionDenied;
    case EADDRINUSE:    return T::AddrInUse;
    case EADDRNOTAVAIL: return T::AddrNotAvailable;
    case ECONNABORTED:  return T::ConnectionAborted;
    case ECONNREFUSED:  return T::ConnectionRefused;
    case ECONNRESET:    return T::ConnectionReset;
    case EEXIST:        return T::AlreadyExists;
    case EINTR:         return T::Interrupted;
    case EINVAL:        return T::InvalidInput;
    case ENOENT:        return T::NotFound;
    case ENOTCONN:      return T::NotConnected;
    case EPIPE:         return T::BrokenPipe;
    case ETIMEDOUT:     return T::TimedOut;
    case EWOULDBLOCK:   return T::WouldBlock;
    default:            return T::Other;
  }
}

static inline void close(int fd) {
  if (fd == -1) {
    return;
  }
  ::close(fd);
}

static inline void flush(int fd) {
  if (fd == -1) {
    return;
  }
  ::fsync(fd);
}

static inline auto read(int fd, void* buf, size_t buf_size) -> ssize_t {
  if (buf == nullptr || buf_size == 0) {
    return 0;
  }
  if (fd == -1) {
    return -1;
  }

  return ::read(fd, buf, buf_size);
}

static inline auto write(int fd, const void* buf, size_t buf_size) -> ssize_t {
  if (fd == -1) {
    return -1;
  }
  if (buf == nullptr || buf_size == 0) {
    return 0;
  }

  return ::write(fd, buf, buf_size);
}

static inline auto is_tty(int fd) -> bool {
  if (fd == -1) {
    return false;
  }
  return ::isatty(fd) != 0;
}

static inline auto stdin() -> int {
  return STDIN_FILENO;
}

static inline auto stdout() -> int {
  return STDOUT_FILENO;
}

static inline auto stderr() -> int {
  return STDERR_FILENO;
}

}  // namespace sfc::sys::io
#endif
