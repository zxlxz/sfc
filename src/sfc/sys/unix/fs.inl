#pragma once

#include "sfc/sys/unix/io.inl"

namespace sfc::sys::unix {

using stat_t = struct stat;

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

  auto is_terminal() const -> bool {
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

struct OpenOptions {
  bool _append = false;
  bool _create = false;
  bool _create_new = false;
  bool _read = false;
  bool _write = false;
  bool _truncate = false;
  mode_t _mode = 0666;

 public:
  auto open(const char* path) const -> io::Result<int> {
    const auto access_flags = _write ? _read ? O_RDWR : O_WRONLY : O_RDONLY;
    const auto create_flags = _create_new ? O_CREAT | O_EXCL : (_create ? O_CREAT : 0);
    const auto append_flags = _append ? O_APPEND : 0;
    const auto truncate_flags = _truncate ? O_TRUNC : 0;

    const auto flag = access_flags | create_flags | append_flags | truncate_flags;
    const auto fd = ::open(path, flag, _mode);
    if (fd == -1) {
      return io::last_os_error();
    }
    return fd;
  }
};

struct Metadata {
  uint32_t _attr;
  size_t _size;

 public:
  auto is_dir() const -> bool {
    return S_ISDIR(_attr);
  }

  auto is_file() const -> bool {
    return S_ISREG(_attr);
  }
};

static inline auto lstat(const char* path) -> io::Result<Metadata> {
  struct stat st{};
  if (::lstat(path, &st) == -1) {
    return io::last_os_error();
  }
  return Metadata{st.st_mode, static_cast<size_t>(st.st_size)};
}

static inline auto unlink(const char* path) -> io::Result<> {
  const auto ret = ::unlink(path);
  if (ret == -1) {
    return io::last_os_error();
  }
  return {};
}

static inline auto rename(const char* old_path, const char* new_path) -> io::Result<> {
  const auto ret = ::rename(old_path, new_path);
  if (ret == -1) {
    return io::last_os_error();
  }
  return {};
}

static inline auto mkdir(const char* path) -> io::Result<> {
  const auto ret = ::mkdir(path, 0755);
  if (ret == -1) {
    return io::last_os_error();
  }
  return {};
}

static inline auto rmdir(const char* path) -> io::Result<> {
  const auto ret = ::rmdir(path);
  if (ret == -1) {
    return io::last_os_error();
  }
  return {};
}

}  // namespace sfc::sys::unix
