#pragma once

#include "sfc/sys/posix/io.inl"

namespace sfc::sys::posix {

using stat_t = struct stat;

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
      return {io::last_os_error()};
    }
    return Ok{fd};
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
    return {io::last_os_error()};
  }
  const auto meta = Metadata{
      st.st_mode,
      num::cast_unsigned(st.st_size),
  };
  return Ok{meta};
}

static inline auto unlink(const char* path) -> io::Result<> {
  if (::unlink(path) == -1) {
    return {io::last_os_error()};
  }
  return Ok{};
}

static inline auto rename(const char* old_path, const char* new_path) -> io::Result<> {
  if (::rename(old_path, new_path) == -1) {
    return {io::last_os_error()};
  }
  return Ok{};
}

static inline auto mkdir(const char* path) -> io::Result<> {
  if (::mkdir(path, 0755) == -1) {
    return {io::last_os_error()};
  }
  return Ok{};
}

static inline auto rmdir(const char* path) -> io::Result<> {
  if (::rmdir(path) == -1) {
    return {io::last_os_error()};
  }
  return Ok{};
}

}  // namespace sfc::sys::posix
