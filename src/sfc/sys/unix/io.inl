#pragma once

#include "sfc/sys/unix/mod.inl"
#define _SFC_SYS_IO_

#include "sfc/io/error.h"

namespace sfc::sys::unix {

static inline auto io_error(int code) -> io::ErrorKind {
  switch (code) {
    case 0:             return io::ErrorKind::Success;
    case ENOENT:        return io::ErrorKind::NotFound;
    case EACCES:
    case EPERM:         return io::ErrorKind::PermissionDenied;
    case ECONNREFUSED:  return io::ErrorKind::ConnectionRefused;
    case ECONNRESET:    return io::ErrorKind::ConnectionReset;
    case ECONNABORTED:  return io::ErrorKind::ConnectionAborted;
    case ENOTCONN:      return io::ErrorKind::NotConnected;
    case EADDRINUSE:    return io::ErrorKind::AddrInUse;
    case EADDRNOTAVAIL: return io::ErrorKind::AddrNotAvailable;
    case ENETUNREACH:   return io::ErrorKind::NetworkUnreachable;
    case EHOSTUNREACH:  return io::ErrorKind::HostUnreachable;
    case ENETDOWN:      return io::ErrorKind::NetworkDown;
    case EPIPE:         return io::ErrorKind::BrokenPipe;
    case EEXIST:        return io::ErrorKind::AlreadyExists;
    case EWOULDBLOCK:   return io::ErrorKind::WouldBlock;
    case EINVAL:        return io::ErrorKind::InvalidInput;
    case EBADMSG:       return io::ErrorKind::InvalidData;
    case ENOTSUP:       return io::ErrorKind::InvalidOperation;
    case EINTR:         return io::ErrorKind::Interrupted;
    case ENOSYS:        return io::ErrorKind::Unsupported;
    case ENOSPC:        return io::ErrorKind::WriteZero;
    case ETIMEDOUT:     return io::ErrorKind::TimedOut;
    case EISDIR:        return io::ErrorKind::IsADirectory;
    case ENOTDIR:       return io::ErrorKind::NotADirectory;
    case ENOTEMPTY:     return io::ErrorKind::DirectoryNotEmpty;
    case ESPIPE:        return io::ErrorKind::NotSeekable;
    case EFBIG:         return io::ErrorKind::FileTooLarge;
    case EBUSY:         return io::ErrorKind::ResourceBusy;
    case EDEADLK:       return io::ErrorKind::Deadlock;
    case ENOMEM:        return io::ErrorKind::OutOfMemory;
    case EINPROGRESS:   return io::ErrorKind::InProgress;
    default:            return io::ErrorKind::Other;
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
