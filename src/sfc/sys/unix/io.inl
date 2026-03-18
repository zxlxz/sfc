#pragma once

#include "sfc/sys/unix/mod.inl"
#define _SFC_SYS_IO_

namespace sfc::sys::unix {

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

struct StdIo {
  int _fd;

 public:
  auto is_console() -> bool {
    return ::isatty(_fd) == 1;
  }

  auto read(Slice<u8> buf) -> io::Result<usize> {
    const auto ret = ::read(_fd, buf._ptr, buf._len);
    if (ret == -1) {
      return io::last_os_error();
    }
    return ret;
  }

  auto write(Slice<const u8> buf) -> io::Result<usize> {
    const auto ret = ::write(_fd, buf._ptr, buf._len);
    if (ret == -1) {
      return io::last_os_error();
    }
    return ret;
  }
};

struct StdIn {
  static auto read(Slice<u8> data) -> io::Result<usize> {
    return StdIo{STDIN_FILENO}.read(data);
  }
};

struct Stdout {
  static auto is_console() -> bool {
    return StdIo{STDOUT_FILENO}.is_console();
  }

  static auto write(Slice<const u8> data) -> io::Result<usize> {
    return StdIo{STDOUT_FILENO}.write(data);
  }

  static auto flush() -> io::Result<> {
    return {};
  }
};

struct Stderr {
  static auto is_console() -> bool {
    return StdIo{STDERR_FILENO}.is_console();
  }

  static auto write(Slice<const u8> data) -> io::Result<usize> {
    return StdIo{STDERR_FILENO}.write(data);
  }

  static auto flush() -> io::Result<> {
    return {};
  }
};

}  // namespace sfc::sys::unix
