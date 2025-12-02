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
static inline auto map_err(int code) -> T {
  switch (code) {
    case 0:                    return T::Success;
    case ENOENT:               return T::NotFound;
    case EACCES:
    case EPERM:                return T::PermissionDenied;
    case ECONNREFUSED:         return T::ConnectionRefused;
    case ECONNRESET:           return T::ConnectionReset;
    case ECONNABORTED:         return T::ConnectionAborted;
    case ENOTCONN:             return T::NotConnected;
    case EADDRINUSE:           return T::AddrInUse;
    case EADDRNOTAVAIL:        return T::AddrNotAvailable;
    case ENETUNREACH:          return T::NetworkUnreachable;
    case EHOSTUNREACH:         return T::HostUnreachable;
    case ENETDOWN:             return T::NetworkDown;
    case EPIPE:                return T::BrokenPipe;
    case EEXIST:               return T::AlreadyExists;
    case EAGAIN:
    case EWOULDBLOCK:          return T::WouldBlock;
    case EINVAL:               return T::InvalidInput;
    case EBADMSG:              return T::InvalidData;
    case ENOTSUP:              return T::InvalidOperation;
    case EINTR:                return T::Interrupted;
    case ENOSYS:               return T::Unsupported;
    case ENOSPC:               return T::WriteZero;
    case ETIMEDOUT:            return T::TimedOut;
    case EISDIR:               return T::IsADirectory;
    case ENOTDIR:              return T::NotADirectory;
    case ENOTEMPTY:            return T::DirectoryNotEmpty;
    case ESPIPE:               return T::NotSeekable;
    case EFBIG:                return T::FileTooLarge;
    case EBUSY:                return T::ResourceBusy;
    case EDEADLK:              return T::Deadlock;
    case ENOSPC:               return T::StorageFull;
    case ENOMEM:               return T::OutOfMemory;
    case EINPROGRESS:          return T::InProgress;
    default:                   return T::Other;
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
