#pragma once

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sfc/io/mod.h"

#undef stdin
#undef stdout
#undef stderr
#undef errno

namespace sfc::sys::io {

using sfc::io::ErrorKind;
using stat_t = struct stat;

struct Error {
  int _code = 0;

 public:
  static auto last() -> Error {
#ifdef __APPLE__
    return {*__error()};
#else
    return {*__errno_location()};
#endif
  }

  auto code() const -> int {
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
  static constexpr int INVALID_FD = -1;

  int _fd = INVALID_FD;

 public:
  explicit operator bool() const {
    return _fd != INVALID_FD;
  }

  void close() {
    if (_fd == INVALID_FD) {
      return;
    }
    ::close(_fd);
  }

  void flush() {
    if (_fd == INVALID_FD) {
      return;
    }
    ::fsync(_fd);
  }

  auto read(void* buf, size_t buf_size) -> ssize_t {
    if (buf == nullptr || buf_size == 0) {
      return 0;
    }
    if (_fd == INVALID_FD) {
      return -1;
    }

    const auto ret = ::read(_fd, buf, buf_size);
    return ret;
  }

  auto write(const void* buf, size_t buf_size) -> ssize_t {
    if (_fd == INVALID_FD) {
      return -1;
    }
    if (buf == nullptr || buf_size == 0) {
      return 0;
    }

    const auto ret = ::write(_fd, buf, buf_size);
    return ret;
  }

  auto is_tty() const -> bool {
    if (_fd == INVALID_FD) {
      return false;
    }
    return ::isatty(_fd) != 0;
  }
};

struct OpenOptions {
  static const auto kFileMode = 0666;

  bool append = false;
  bool create = false;
  bool create_new = false;
  bool read = false;
  bool write = false;
  bool truncate = false;

 public:
  static auto from(const auto& t) -> OpenOptions {
    return OpenOptions{t._append, t._create, t._create_new, t._read, t._write, t._truncate};
  }

  auto open(const char* path) const -> int {
    // access mode
    int access_mode = 0;
    if (read && write) {
      access_mode |= O_RDWR;
    } else if (read) {
      access_mode |= O_RDONLY;
    } else if (write) {
      access_mode |= O_WRONLY;
    }

    // creation mode
    int create_mode = 0;
    if (create || create_new) {
      create_mode |= O_CREAT;
    }

    if (create_new) {
      create_mode |= O_EXCL;
    }

    if (truncate) {
      create_mode |= O_TRUNC;
    }

    if (append) {
      create_mode |= O_APPEND;
    }

    const auto fd = ::open(path, access_mode | create_mode, kFileMode);
    return fd;
  }
};

struct FileAttr {
  usize _attr;
  usize _size;

 public:
  operator bool() const {
    return _attr != 0;
  }

  auto is_dir() const -> bool {
    return S_ISDIR(_attr);
  }

  auto is_file() const -> bool {
    return S_ISREG(_attr);
  }

  auto is_symlink() const -> bool {
    return S_ISLNK(_attr);
  }
};

static inline auto stdin() -> File {
  return File{0};
}

static inline auto stdout() -> File {
  return File{1};
}

static inline auto stderr() -> File {
  return File{2};
}

static inline auto lstat(const char* path) -> FileAttr {
  auto       st = stat_t{};
  const auto ret = ::lstat(path, &st);
  if (ret == -1) {
    return FileAttr{0, 0};
  }
  return FileAttr{st.st_mode, static_cast<usize>(st.st_size)};
};

static inline auto unlink(const char* path) -> bool {
  return ::unlink(path) == 0;
}

static inline auto rename(const char* old_path, const char* new_path) -> bool {
  const auto ret = ::rename(old_path, new_path);
  return ret == 0;
}

static inline auto mkdir(const char* path) -> bool {
  static constexpr auto kMode = 0777;

  const auto ret = ::mkdir(path, kMode);
  return ret == 0;
}

static inline auto rmdir(const char* path) -> bool {
  const auto ret = ::rmdir(path);
  return ret == 0;
}

}  // namespace sfc::sys::io
