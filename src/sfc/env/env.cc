#include "sfc/env.h"
#include "sfc/ffi/os_str.h"
#include "sfc/sys/env.h"

namespace sfc::env {

using ffi::OsString;

auto var(Str key) -> String {
  const auto os_key = OsString::from_str(key);
  return sys::getenv(os_key.as_ptr());
}

auto set_var(Str key, Str val) -> bool {
  const auto os_key = OsString::from_str(key);
  const auto os_val = OsString::from_str(val);
  return sys::setenv(os_key.as_ptr(), os_val.as_ptr());
}

auto remove_var(Str key) -> bool {
  const auto os_key = OsString::from_str(key);
  return sys::unsetenv(os_key.as_ptr());
}

auto home_dir() -> fs::PathBuf {
  auto s = sys::home_dir();
  return fs::PathBuf::from(mem::move(s));
}

auto temp_dir() -> fs::PathBuf {
  auto s = sys::temp_dir();
  return fs::PathBuf::from(mem::move(s));
}

auto current_exe() -> fs::PathBuf {
  auto s = sys::current_exe();
  return fs::PathBuf::from(mem::move(s));
}

auto current_dir() -> fs::PathBuf {
  auto s = sys::getcwd();
  return fs::PathBuf::from(mem::move(s));
}

auto set_current_dir(fs::Path path) -> bool {
  const auto os_path = OsString::from_str(path.as_str());
  const auto ret = sys::chdir(os_path.as_ptr());
  return ret;
}

}  // namespace sfc::env
