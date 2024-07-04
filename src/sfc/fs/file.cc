#include "sfc/fs/file.h"

#include "sfc/sys/fs.h"

namespace sfc::fs {

namespace sys_imp = sys::fs;

auto File::open(const Path& path) -> io::Result<File> {
  const auto opts = OpenOptions{.read = true};
  return opts.open(path);
}

auto File::create(const Path& path) -> io::Result<File> {
  const auto opts = OpenOptions{.create = true, .write = true, .truncate = true};
  return opts.open(path);
}

auto OpenOptions::open(const Path& path) const -> io::Result<File> {
  const auto c_path = CString::from(path.as_str());

  auto file = io::File{sys_imp::open(c_path, *this)};
  if (!file) {
    return io::Error::last_os_error();
  }

  return File{static_cast<io::File&&>(file)};
}

}  // namespace sfc::fs
