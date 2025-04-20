
#include "funcs.h"

#include "sfc/ffi/cstring.h"
#include "sfc/sys/io.h"

namespace sfc::fs {

namespace sys_imp = sys::io;

auto create_dir(const Path& path) -> io::Result<> {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto ret = sys_imp::mkdir(os_path.c_str());

  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

auto remove_dir(const Path& path) -> io::Result<> {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto ret = sys_imp::rmdir(os_path.c_str());
  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

auto remove_file(const Path& path) -> io::Result<> {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto ret = sys_imp::unlink(os_path.c_str());
  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

auto rename(const Path& old_path, const Path& new_path) -> io::Result<> {
  const auto os_old_path = ffi::CString::from(old_path.as_str());
  const auto os_new_path = ffi::CString::from(new_path.as_str());

  const auto ret = sys_imp::rename(os_old_path.c_str(), os_new_path.c_str());
  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

}  // namespace sfc::fs
