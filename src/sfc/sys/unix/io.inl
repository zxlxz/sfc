#pragma once

#include "sfc/sys/unix/mod.inl"
#define _SFC_SYS_IO_

namespace sfc::sys::unix {

class File {
  int _fd = -1;

 public:
  File(int fd = -1) : _fd(fd) {}

  ~File() {
    if (_fd == -1) return;
    ::close(_fd);
  }

  File(File&& other) noexcept : _fd(other._fd) {
    other._fd = -1;
  }

  File& operator=(File&& other) noexcept {
    if (this != &other) {
      mem::swap(_fd, other._fd);
    }
    return *this;
  }

  auto is_valid() const -> bool {
    return _fd != -1;
  }

  auto is_tty() const -> bool {
    const auto ret = ::isatty(_fd);
    return ret != 0;
  }

  auto flush() -> io::Result<> {
    const auto ret = ::fsync(_fd);
    if (ret == -1) {
      return io::last_os_error();
    }
    return {};
  }

  auto read(Slice<u8> buf) -> io::Result<usize> {
    const auto ret = ::read(_fd, buf._ptr, buf._len);
    if (ret == -1) {
      return io::last_os_error();
    }
    return static_cast<usize>(ret);
  }

  auto write(Slice<const u8> buf) -> io::Result<usize> {
    const auto ret = ::write(_fd, buf._ptr, buf._len);
    if (ret == -1) {
      return io::last_os_error();
    }
    return static_cast<usize>(ret);
  }

  auto seek(off_t offset, int whence) -> io::Result<usize> {
    static_assert(SEEK_SET == 0);
    static_assert(SEEK_CUR == 1);
    static_assert(SEEK_END == 2);
    const auto ret = ::lseek(_fd, offset, whence);
    if (ret == -1) {
      return io::last_os_error();
    }
    return static_cast<usize>(ret);
  }
};

static inline auto stdin() -> File& {
  static auto res = File{STDIN_FILENO};
  return res;
}

static inline auto stdout() -> File& {
  static auto res = File{STDOUT_FILENO};
  return res;
}

static inline auto stderr() -> File& {
  static auto res = File{STDERR_FILENO};
  return res;
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

}  // namespace sfc::sys::unix
