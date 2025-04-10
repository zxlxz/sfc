
#include "funcs.h"

#include "sfc/sys/io.h"

namespace sfc::fs {

namespace sys_imp = sys::io;

auto create_dir(const Path& path) -> io::Result<> {
  const auto ret = sys_imp::mkdir(path.c_str());
  if (!ret) {
    return io::Error::last_os_error();
  }
  
  return _;
}

auto remove_dir(const Path& path) -> io::Result<> {
  const auto ret = sys_imp::rmdir(path.c_str());
  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

auto remove_file(const Path& path) -> io::Result<> {
  const auto ret = sys_imp::unlink(path.c_str());
  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

auto rename(const Path& old_path, const Path& new_path) -> io::Result<> {
  const auto ret = sys_imp::rename(old_path.c_str(), new_path.c_str());
  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

}  // namespace sfc::fs
