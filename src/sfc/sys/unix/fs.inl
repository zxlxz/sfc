#pragma once

#include "sfc/sys/unix/io.inl"

namespace sfc::sys::unix {

using stat_t = struct stat;

struct OpenOptions {
  bool _append = false;
  bool _create = false;
  bool _create_new = false;
  bool _read = false;
  bool _write = false;
  bool _truncate = false;
  mode_t _mode = 0666;

  auto open(const char* path) const -> File {
    const auto access_flags = _write ? _read ? O_RDWR : O_WRONLY : O_RDONLY;
    const auto create_flags = _create_new ? O_CREAT | O_EXCL : (_create ? O_CREAT : 0);
    const auto append_flags = _append ? O_APPEND : 0;
    const auto truncate_flags = _truncate ? O_TRUNC : 0;

    const auto flag = access_flags | create_flags | append_flags | truncate_flags;
    const auto fd = ::open(path, flag, _mode);
    return File{fd};
  }
};

static inline auto is_dir(int attr) -> bool {
  return (attr & S_IFMT) == S_IFDIR;
}

static inline auto is_file(int attr) -> bool {
  return (attr & S_IFMT) == S_IFREG;
}

template<class Meta>
static inline auto lstat(const char* path) -> io::Result<Meta> {
  auto st = stat_t{};
  if (::lstat(path, &st) == -1) {
    return io::last_os_error();
  }

  const auto res = Meta{
      ._attr = static_cast<unsigned>(st.st_mode),
      ._size = static_cast<size_t>(st.st_size),
  };
  return res;
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
