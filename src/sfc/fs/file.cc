#include "file.h"

#include "sfc/sys/fs.inl"

namespace sfc::fs {

namespace sys_imp = sys::fs;

auto OpenOptions::open(Path path) const -> io::Result<File> {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto sys_imp = sys_imp::OpenOptions{*this};

  auto imp = sys_imp.open(os_path);
  if (!imp) {
    return io::Error::last_os_error();
  }
  return File{imp};
}

auto File::open(Path path) -> io::Result<File> {
  auto opts = OpenOptions{};
  opts._read = true;
  return opts.open(path);
}

auto File::create(Path path) -> io::Result<File> {
  auto opts = OpenOptions{};
  opts._write = true;
  opts._create = true;
  opts._truncate = true;

  return opts.open(path);
}

}  // namespace sfc::fs
