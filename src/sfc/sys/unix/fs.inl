#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::fs {

using stat_t = struct stat;
static const auto INVALID_FD = -1;

static inline auto open(const char* path, const auto& opts) -> int {
  const auto access_flags = opts.write ? opts.read ? O_RDWR : O_WRONLY : O_RDONLY;
  const auto create_flags = opts.create_new ? O_CREAT | O_EXCL : (opts.create ? O_CREAT : 0);
  const auto append_flags = opts.append ? O_APPEND : 0;
  const auto truncate_flags = opts.truncate ? O_TRUNC : 0;

  const auto flag = access_flags | create_flags | append_flags | truncate_flags;
  const auto mode = 0666;
  return ::open(path, flag, mode);
}

static inline auto is_dir(int attr) -> bool {
  return (attr & S_IFMT) == S_IFDIR;
}

static inline auto is_file(int attr) -> bool {
  return (attr & S_IFMT) == S_IFREG;
}

static inline auto lstat(const char* path, auto& res) -> bool {
  auto st = stat_t{};
  if (::lstat(path, &st) != 0) {
    return false;
  }

  res._attr = static_cast<unsigned>(st.st_mode);
  res._size = static_cast<size_t>(st.st_size);
  return true;
}

static inline auto unlink(const char* path) -> bool {
  return ::unlink(path) == 0;
}

static inline auto rename(const char* old_path, const char* new_path) -> bool {
  return ::rename(old_path, new_path) == 0;
}

static inline auto mkdir(const char* path) -> bool {
  return ::mkdir(path, 0755) == 0;
}

static inline auto rmdir(const char* path) -> bool {
  return ::rmdir(path) == 0;
}

}  // namespace sfc::sys::.fs
