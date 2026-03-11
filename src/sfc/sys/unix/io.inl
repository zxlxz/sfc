#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

struct File {
  int _fd = -1;

 public:
  auto is_valid() const -> bool {
    return _fd != -1;
  }

  auto close() -> bool {
    if (_fd == -1) {
      return 0;
    }
    const auto ret = ::close(_fd);
    _fd = -1;
    return ret != -1;
  }

  auto flush() -> bool {
    if (_fd == -1) {
      return true;
    }
    const auto ret = ::fsync(_fd);
    return ret != -1;
  }

  auto read(void* buf, size_t buf_size) -> ssize_t {
    if (buf == nullptr || buf_size == 0) {
      return 0;
    }
    return ::read(_fd, buf, buf_size);
  }

  auto write(const void* buf, size_t buf_size) -> ssize_t {
    if (buf == nullptr || buf_size == 0) {
      return 0;
    }
    return ::write(_fd, buf, buf_size);
  }

  auto seek(off_t offset, int whence) -> off_t {
    static_assert(SEEK_SET == 0);
    static_assert(SEEK_CUR == 1);
    static_assert(SEEK_END == 2);

    const auto ret = ::lseek(_fd, offset, whence);
    return ret;
  }

  auto is_tty() const -> bool {
    const auto ret = ::isatty(_fd);
    return ret != 0;
  }
};

static inline auto stdin() -> File {
  return File{STDIN_FILENO};
}

static inline auto stdout() -> File {
  return File{STDOUT_FILENO};
}

static inline auto stderr() -> File {
  return File{STDERR_FILENO};
}

static inline auto io_error(int code) -> io::Error {
  switch (code) {
    case 0:             return io::Error::Success;
    case ENOENT:        return io::Error::NotFound;
    case EACCES:
    case EPERM:         return io::Error::PermissionDenied;
    case ECONNREFUSED:  return io::Error::ConnectionRefused;
    case ECONNRESET:    return io::Error::ConnectionReset;
    case ECONNABORTED:  return io::Error::ConnectionAborted;
    case ENOTCONN:      return io::Error::NotConnected;
    case EADDRINUSE:    return io::Error::AddrInUse;
    case EADDRNOTAVAIL: return io::Error::AddrNotAvailable;
    case ENETUNREACH:   return io::Error::NetworkUnreachable;
    case EHOSTUNREACH:  return io::Error::HostUnreachable;
    case ENETDOWN:      return io::Error::NetworkDown;
    case EPIPE:         return io::Error::BrokenPipe;
    case EEXIST:        return io::Error::AlreadyExists;
    case EWOULDBLOCK:   return io::Error::WouldBlock;
    case EINVAL:        return io::Error::InvalidInput;
    case EBADMSG:       return io::Error::InvalidData;
    case ENOTSUP:       return io::Error::InvalidOperation;
    case EINTR:         return io::Error::Interrupted;
    case ENOSYS:        return io::Error::Unsupported;
    case ENOSPC:        return io::Error::WriteZero;
    case ETIMEDOUT:     return io::Error::TimedOut;
    case EISDIR:        return io::Error::IsADirectory;
    case ENOTDIR:       return io::Error::NotADirectory;
    case ENOTEMPTY:     return io::Error::DirectoryNotEmpty;
    case ESPIPE:        return io::Error::NotSeekable;
    case EFBIG:         return io::Error::FileTooLarge;
    case EBUSY:         return io::Error::ResourceBusy;
    case EDEADLK:       return io::Error::Deadlock;
    case ENOMEM:        return io::Error::OutOfMemory;
    case EINPROGRESS:   return io::Error::InProgress;
    default:            return io::Error::Other;
  }
}

}  // namespace sfc::sys
