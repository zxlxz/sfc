#include "sfc/fs/file.h"

#include "sfc/ffi/cstring.h"
#include "sfc/sys/io.h"

namespace sfc::fs {

namespace sys_imp = sys::io;

auto OpenOptions::open(const Path& path) const -> io::Result<File> {
  const auto sys_opt = sys_imp::OpenOptions::from(*this);
  const auto sys_path = ffi::CString::from(path.as_str());

  auto sys_fd = sys_opt.open(sys_path.c_str());
  if (!sys_fd) {
    return io::Error::last_os_error();
  }

  auto res = File{};
  res._fd = sys_fd;

  return res;
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
