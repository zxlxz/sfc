#include "sfc/env.h"
#include "sfc/ffi/os_str.h"
#include "sfc/sys/env.h"

namespace sfc::env {

namespace sys_imp = sys::env;

auto var(Str key) -> String {
  const auto os_key = ffi::OsString::from(key);
  return sys_imp::getenv(os_key.ptr()).into_string();
}

auto set_var(Str key, Str val) -> bool {
  const auto os_key = ffi::OsString::from(key);
  const auto os_val = ffi::OsString::from(val);
  return sys_imp::setenv(os_key.ptr(), os_val.ptr());
}

auto remove_var(Str key) -> bool {
  const auto os_key = ffi::OsString::from(key);
  return sys_imp::unsetenv(os_key.ptr());
}

auto home_dir() -> fs::PathBuf {
  const auto s = sys_imp::home_dir().into_string();
  return fs::PathBuf::from(s.as_str());
}

auto temp_dir() -> fs::PathBuf {
  const auto s = sys_imp::temp_dir().into_string();
  return fs::PathBuf::from(s.as_str());
}

auto current_exe() -> fs::PathBuf {
  const auto s = sys_imp::current_exe().into_string();
  return fs::PathBuf::from(s.as_str());
}

auto current_dir() -> fs::PathBuf {
  const auto s = sys_imp::getcwd().into_string();
  return fs::PathBuf::from(s.as_str());
}

auto set_current_dir(fs::Path path) -> bool {
  const auto os_path = ffi::OsString::from(path.as_str());
  const auto ret = sys_imp::chdir(os_path.ptr());
  return ret;
}

}  // namespace sfc::env
