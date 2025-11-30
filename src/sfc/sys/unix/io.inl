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
    case 0:             return T::Success;
    case ENOENT:        return T::NotFound;
    case EACCES:
    case EPERM:         return T::PermissionDenied;
    case ECONNREFUSED:  return T::ConnectionRefused;
    case ECONNRESET:    return T::ConnectionReset;
    case EHOSTUNREACH:
    case EHOSTDOWN:     return T::HostUnreachable;
    case ENETUNREACH:
    case ENETRESET:     return T::NetworkUnreachable;
    case ECONNABORTED:  return T::ConnectionAborted;
    case ENOTCONN:      return T::NotConnected;
    case EADDRINUSE:    return T::AddrInUse;
    case EADDRNOTAVAIL: return T::AddrNotAvailable;
    case ENETDOWN:      return T::NetworkDown;
    case EPIPE:         return T::BrokenPipe;
    case EEXIST:        return T::AlreadyExists;
    case EWOULDBLOCK:   return T::WouldBlock;
    case ENOTDIR:       return T::NotADirectory;
    case EISDIR:        return T::IsADirectory;
    case ENOTEMPTY:     return T::DirectoryNotEmpty;
    case EROFS:         return T::ReadOnlyFilesystem;
    case ELOOP:         return T::FilesystemLoop;
    case ESTALE:        return T::StaleNetworkFileHandle;
    case EINVAL:
    case EBADF:
    case EFAULT:        return T::InvalidInput;
    case EILSEQ:        return T::InvalidData;
    case ETIMEDOUT:     return T::TimedOut;
    case EIO:           return T::WriteZero;
    case ENOSPC:        return T::StorageFull;
    case ESPIPE:        return T::NotSeekable;
    case EDQUOT:        return T::QuotaExceeded;
    case EFBIG:         return T::FileTooLarge;
    case EBUSY:         return T::ResourceBusy;
    case ETXTBSY:       return T::ExecutableFileBusy;
    case EDEADLK:       return T::Deadlock;
    case EXDEV:         return T::CrossesDevices;
    case EMLINK:        return T::TooManyLinks;
    case ENAMETOOLONG:  return T::InvalidFilename;
    case E2BIG:         return T::ArgumentListTooLong;
    case EINTR:         return T::Interrupted;
    case ENOSYS:
    case ENOTSUP:
    case EOPNOTSUPP:    return T::Unsupported;
    case ENODATA:       return T::UnexpectedEof;
    case ENOMEM:        return T::OutOfMemory;
    case EINPROGRESS:   return T::InProgress;
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
