#include "env.h"

#include "sfc/ffi.h"
#include "sfc/sys/env.inl"

namespace sfc::env {

namespace sys_imp = sys::env;

auto var(Str key) -> Str {
  const auto c_key = ffi::CString::from(key);
  return sys_imp::getenv(c_key);
}

void set_var(Str key, Str val) {
  const auto c_key = ffi::CString::from(val);
  const auto c_val = ffi::CString::from(val);

  const auto ret = sys_imp::setenv(c_key, c_val);
  assert_fmt(ret, "env::set: key=`{}`, val=`{}`, err={}", key, val, io::Error::last_os_error());
}

void remove_var(Str key) {
  const auto c_key = ffi::CString::from(key);
  sys_imp::unsetenv(c_key);
}

auto current_dir() -> fs::PathBuf {
  const auto res = sys_imp::getcwd();
  return fs::PathBuf::from(res);
}

void set_current_dir(fs::Path path) {
  const auto os_path = ffi::CString::from(path.as_str());
  const auto ret = sys_imp::chdir(os_path);
  assert_fmt(ret, "env::set_current_dir: path=`{}`, err=`{}`", path, io::Error::last_os_error());
}

auto current_exe() -> fs::PathBuf {
  const auto res = sys_imp::current_exe();
  return fs::PathBuf::from(res);
}

auto home_dir() -> fs::PathBuf {
  const auto res = sys_imp::home_dir();
  return fs::PathBuf::from(res);
}

}  // namespace sfc::env
