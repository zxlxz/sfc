
#include "funcs.h"

#include "sfc/ffi.h"
#include "sfc/sys/fs.inl"

namespace sfc::fs {

namespace sys_imp = sys::fs;

auto create_dir(Path path) -> io::Result<> {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto ret = sys_imp::mkdir(os_path);
  if (!ret) {
    return io::Error::last_os_error();
  }
  return _;
}

auto remove_dir(Path path) -> io::Result<> {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto ret = sys_imp::rmdir(os_path);
  if (!ret) {
    return io::Error::last_os_error();
  }
  return _;
}

auto remove_file(Path path) -> io::Result<> {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto ret = sys_imp::unlink(os_path);
  if (!ret) {
    return io::Error::last_os_error();
  }
  return _;
}

auto rename(Path old_path, Path new_path) -> io::Result<> {
  const auto os_old = ffi::CString::from(old_path.as_str());
  const auto os_new = ffi::CString::from(new_path.as_str());
  const auto ret = sys_imp::rename(os_old, os_new);
  if (!ret) {
    return io::Error::last_os_error();
  }
  return _;
}

auto symlink(Path original, Path link) -> io::Result<> {
  const auto os_orig = ffi::CString::from(original.as_str());
  const auto os_link = ffi::CString::from(link.as_str());
  const auto ret = sys_imp::symlink(os_orig, os_link);
  if (!ret) {
    return io::Error::last_os_error();
  }
  return _;
}

}  // namespace sfc::fs
