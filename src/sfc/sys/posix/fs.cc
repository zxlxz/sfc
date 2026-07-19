#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "sfc/sys/posix/fs.h"
#include "sfc/fs/file.h"

namespace sfc::sys::posix {

using stat_t = struct stat;

auto FileAttr::is_dir() const -> bool {
  return S_ISDIR(_attr);
}

auto FileAttr::is_file() const -> bool {
  return S_ISREG(_attr);
}

auto open(const char* path, fs::OpenOptions opts) -> io::Result<RawFd> {
  const auto mode = 0666;

  const auto access_flags = opts.write ? opts.read ? O_RDWR : O_WRONLY : O_RDONLY;
  const auto create_flags = opts.create_new ? O_CREAT | O_EXCL : (opts.create ? O_CREAT : 0);
  const auto append_flags = opts.append ? O_APPEND : 0;
  const auto truncate_flags = opts.truncate ? O_TRUNC : 0;
  const auto flag = access_flags | create_flags | append_flags | truncate_flags;

  const auto fd = ::open(path, flag, mode);
  if (fd == -1) {
    return io::last_os_error();
  }
  return Ok{fd};
}

auto lstat(const char* path) -> io::Result<fs::Metadata> {
  struct stat st{};
  if (::lstat(path, &st) == -1) {
    return io::last_os_error();
  }

  const auto attr = st.st_mode;
  const auto size = num::cast_unsigned(st.st_size);
  const auto meta = fs::Metadata{attr, size};
  return meta;
}

auto unlink(const char* path) -> io::Result<> {
  if (::unlink(path) == -1) {
    return io::last_os_error();
  }
  return Ok{};
}

auto rename(const char* old_path, const char* new_path) -> io::Result<> {
  if (::rename(old_path, new_path) == -1) {
    return io::last_os_error();
  }
  return Ok{};
}

auto mkdir(const char* path) -> io::Result<> {
  static constexpr auto kDirMode = 0755;
  if (::mkdir(path, kDirMode) == -1) {
    return {io::last_os_error()};
  }
  return Ok{};
}

auto rmdir(const char* path) -> io::Result<> {
  if (::rmdir(path) == -1) {
    return io::last_os_error();
  }
  return Ok{};
}

}  // namespace sfc::sys::posix
