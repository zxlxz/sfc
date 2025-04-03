#include "file.h"

#include "sfc/sys/fs.inl"

namespace sfc::fs {

namespace sys_imp = sys::fs;

auto OpenOptions::open(const Path& path) const -> io::Result<File> {
  const auto sys_imp = sys_imp::OpenOptions{*this};

  auto imp = sys_imp.open(path.c_str());
  if (!imp) {
    return io::Error::last_os_error();
  }
  return File{io::File::from_raw_fd(imp._fd)};
}

auto File::open(const Path& path) -> io::Result<File> {
  auto opts = OpenOptions{};
  opts._read = true;
  return opts.open(path);
}

auto File::create(const Path& path) -> io::Result<File> {
  auto opts = OpenOptions{};
  opts._write = true;
  opts._create = true;
  opts._truncate = true;

  return opts.open(path);
}

}  // namespace sfc::fs
