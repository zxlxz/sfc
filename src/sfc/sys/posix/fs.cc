#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "sfc/fs/file.h"

#include "sfc/sys/cstr.inl"
#include "sfc/sys/posix/fs.h"

namespace sfc::sys::posix {

using stat_t = struct stat;

auto FileAttr::exists() const noexcept -> bool {
  return _attr != 0;
}

auto FileAttr::is_dir() const noexcept -> bool {
  return S_ISDIR(_attr);
}

auto FileAttr::is_file() const noexcept -> bool {
  return S_ISREG(_attr);
}

auto FileAttr::is_link() const noexcept -> bool {
  return S_ISLNK(_attr);
}

auto open(Str path, fs::OpenOptions opts) -> io::Result<RawFd> {
  constexpr auto kFileMode = 0666;

  const auto os_path = FixedCStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  const auto access_flags = opts.write ? opts.read ? O_RDWR : O_WRONLY : O_RDONLY;
  const auto create_flags = opts.create_new ? O_CREAT | O_EXCL : (opts.create ? O_CREAT : 0);
  const auto append_flags = opts.append ? O_APPEND : 0;
  const auto truncate_flags = opts.truncate ? O_TRUNC : 0;
  const auto flag = access_flags | create_flags | append_flags | truncate_flags;

  const auto fd = ::open(os_path, flag, kFileMode);
  if (fd == -1) {
    return io::last_os_error();
  }

  return Ok{fd};
}

auto fstat(RawFd fd) -> io::Result<fs::Metadata> {
  struct stat st{};
  if (::fstat(fd, &st) == -1) {
    return io::last_os_error();
  }

  const auto meta = fs::Metadata{u64(st.st_size), st.st_mode};
  return Ok{meta};
}

auto lstat(Str path) -> io::Result<fs::Metadata> {
  const auto os_path = FixedCStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  struct stat st{};
  const auto ret = ::lstat(os_path, &st);
  if (ret == -1) {
    return io::last_os_error();
  }

  const auto attr = st.st_mode;
  const auto size = u64(st.st_size);
  const auto meta = fs::Metadata{size, attr};
  return Ok{meta};
}

auto unlink(Str path) -> io::Result<> {
  const auto os_path = FixedCStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  const auto ret = ::unlink(os_path);
  if (ret == -1) {
    return io::last_os_error();
  }
  return Ok{};
}

auto rename(Str old_path, Str new_path) -> io::Result<> {
  const auto os_old_path = FixedCStr<kMaxPath>::from_str(old_path);
  const auto os_new_path = FixedCStr<kMaxPath>::from_str(new_path);
  if (!os_old_path || !os_new_path) {
    return io::Error::InvalidInput;
  }

  const auto ret = ::rename(os_old_path, os_new_path);
  if (ret == -1) {
    return io::last_os_error();
  }
  return Ok{};
}

auto mkdir(Str path) -> io::Result<> {
  static constexpr auto kDirMode = 0755;

  const auto os_path = FixedCStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  const auto ret = ::mkdir(os_path, kDirMode);
  if (ret == -1) {
    return {io::last_os_error()};
  }

  return Ok{};
}

auto rmdir(Str path) -> io::Result<> {
  const auto os_path = FixedCStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  const auto ret = ::rmdir(os_path);
  if (ret == -1) {
    return io::last_os_error();
  }

  return Ok{};
}

auto ReadDir::open(Str path) -> io::Result<ReadDir> {
  const auto os_path = FixedCStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return io::Error::InvalidInput;
  }

  const auto dir = ::opendir(os_path);
  if (!dir) {
    return io::last_os_error();
  }

  return ReadDir{dir};
}

auto ReadDir::close() -> io::Result<> {
  if (_dir == nullptr) {
    return Ok{};
  }

  const auto ret = ::closedir(_dir);
  if (ret == -1) {
    return io::last_os_error();
  }
  _dir = nullptr;
  return Ok{};
}

auto ReadDir::next() -> Option<DirEntry> {
  const auto dirent = ::readdir(_dir);
  if (!dirent) {
    return None{};
  }

  const auto file_size = 0;  // posix: file_size not support on dirent
  const auto file_attr = u32(DTTOIF(dirent->d_type));

  return DirEntry{
      ._name = dirent->d_name,
      ._size = file_size,
      ._attr = file_attr,
  };
}

}  // namespace sfc::sys::posix
