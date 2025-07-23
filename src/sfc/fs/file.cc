#include "sfc/fs/file.h"

#include "sfc/ffi/cstring.h"
#include "sfc/sys/fs.h"

namespace sfc::fs {

namespace sys_imp = sys::fs;

auto File::open(const Path& path) -> io::Result<File> {
  const auto opts = OpenOptions{.read = true};
  return opts.open(path);
}

auto File::create(const Path& path) -> io::Result<File> {
  const auto opts = OpenOptions{
      .create = true,
      .write = true,
      .truncate = true,
  };

  return opts.open(path);
}

auto OpenOptions::open(const Path& path) const -> io::Result<File> {
  const auto sys_path = ffi::CString::from(path.as_str());

  auto fd = io::fd_t{};
  if (!sys_imp::open(*this, sys_path.c_str(), &fd)) {
    return io::Error::last_os_error();
  }

  return File{fd};
}

}  // namespace sfc::fs
