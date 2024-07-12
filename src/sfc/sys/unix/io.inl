#pragma once

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sfc/io/error.h"

#undef stdin
#undef stdout
#undef stderr
#undef errno

namespace sfc::sys::io {

using fd_t = int;
using sfc::io::ErrorKind;

static constexpr fd_t INVALID_FD = -1;

struct File {
  fd_t _fd = -1;

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

static inline auto err_kind(int code) -> ErrorKind {
  // clang-format off
  switch(code) {
    case E2BIG:         return ErrorKind::ArgumentListTooLong;
    case EADDRINUSE:    return ErrorKind::AddrInUse;
    case EADDRNOTAVAIL: return ErrorKind::AddrNotAvailable;
    case EBUSY:         return ErrorKind::ResourceBusy;
    case ECONNABORTED:  return ErrorKind::ConnectionAborted;
    case ECONNREFUSED:  return ErrorKind::ConnectionRefused;
    case ECONNRESET:    return ErrorKind::ConnectionReset;
    case EDEADLK:       return ErrorKind::Deadlock;
    case EDQUOT:        return ErrorKind::FilesystemQuotaExceeded;
    case EEXIST:        return ErrorKind::AlreadyExists;
    case EFBIG:         return ErrorKind::FileTooLarge;
    case EHOSTUNREACH:  return ErrorKind::HostUnreachable;
    case EINTR:         return ErrorKind::Interrupted;
    case EINVAL:        return ErrorKind::InvalidInput;
    case EISDIR:        return ErrorKind::IsADirectory;
    case ELOOP:         return ErrorKind::FilesystemLoop;
    case ENOENT:        return ErrorKind::NotFound;
    case ENOMEM:        return ErrorKind::OutOfMemory;
    case ENOSPC:        return ErrorKind::StorageFull;
    case ENOSYS:        return ErrorKind::Unsupported;
    case EMLINK:        return ErrorKind::TooManyLinks;
    case ENAMETOOLONG:  return ErrorKind::InvalidFilename;
    case ENETDOWN:      return ErrorKind::NetworkDown;
    case ENETUNREACH:   return ErrorKind::NetworkUnreachable;
    case ENOTCONN:      return ErrorKind::NotConnected;
    case ENOTDIR:       return ErrorKind::NotADirectory;
    case ENOTEMPTY:     return ErrorKind::DirectoryNotEmpty;
    case EPIPE:         return ErrorKind::BrokenPipe;
    case EROFS:         return ErrorKind::ReadOnlyFilesystem;
    case ESPIPE:        return ErrorKind::NotSeekable;
    case ESTALE:        return ErrorKind::StaleNetworkFileHandle;
    case ETIMEDOUT:     return ErrorKind::TimedOut;
    case ETXTBSY:       return ErrorKind::ExecutableFileBusy;
    case EXDEV:         return ErrorKind::CrossesDevices;
    case EACCES:        return ErrorKind::PermissionDenied;
    case EPERM:         return ErrorKind::PermissionDenied;
    case EWOULDBLOCK:   return ErrorKind::WouldBlock;
    default:            return ErrorKind::Other;
  }
  // clang-format on
}

}  // namespace sfc::sys::io
