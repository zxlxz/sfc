#include "file.h"

#include "sfc/sys/fs.inl"

namespace sfc::fs {

namespace sys_imp = sys::fs;

auto OpenOptions::open(Path path) const -> File {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto sys_imp = sys_imp::OpenOptions{*this};

  auto file = File{sys_imp.open(os_path)};
  return file;
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
