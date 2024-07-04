#include "file.h"

#include <fcntl.h>

namespace sfc::fs {

using ffi::CString;

auto OpenOptions::open(Path path) const -> File {
  const auto access_mode = [&] {
    const auto a = _write ? _read ? O_RDWR : O_WRONLY : _append ? O_RDWR : O_RDONLY;
    const auto b = _append ? O_APPEND : 0;
    return a | b;
  }();

  const auto create_mode = [&] {
    const auto a = _create ? O_CREAT : _create_new ? O_CREAT : 0;
    const auto b = _create_new ? O_EXCL : 0;
    const auto c = _truncate ? O_TRUNC : 0;
    return a | b | c;
  }();

  const auto flag = access_mode | create_mode;

  const auto fd = ::open(path.to_c_string(), O_CLOEXEC | flag, 0666);
  return File{fd};
}

auto File::open(Path path) -> File {
  auto opts = OpenOptions{};
  opts._read = true;
  auto file = opts.open(path);
  assert_fmt(file, "fs::File::open(path='{}'): {}", path, io::Error::last_os_error());
  return file;
}

auto File::create(Path path) -> File {
  auto opts = OpenOptions{};
  opts._write = true;
  opts._create = true;
  opts._truncate = true;

  auto file = opts.open(path);
  assert_fmt(file, "fs::File::create(path='{}'): {}", path, io::Error::last_os_error());
  return file;
}

}  // namespace sfc::fs
