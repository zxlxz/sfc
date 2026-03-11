#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/env.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/env.inl"
#endif

#include "sfc/env.h"
#include "sfc/ffi/os_str.h"

namespace sfc::env {

auto var(Str key) -> String {
  const auto os_key = ffi::OsString::from(key);
  return sys::getenv(os_key.ptr()).into_string();
}

auto set_var(Str key, Str val) -> bool {
  const auto os_key = ffi::OsString::from(key);
  const auto os_val = ffi::OsString::from(val);
  return sys::setenv(os_key.ptr(), os_val.ptr());
}

auto remove_var(Str key) -> bool {
  const auto os_key = ffi::OsString::from(key);
  return sys::unsetenv(os_key.ptr());
}

auto home_dir() -> fs::PathBuf {
  const auto s = sys::home_dir().into_string();
  return fs::PathBuf::from(s.as_str());
}

auto temp_dir() -> fs::PathBuf {
  const auto s = sys::temp_dir().into_string();
  return fs::PathBuf::from(s.as_str());
}

auto current_exe() -> fs::PathBuf {
  const auto s = sys::current_exe().into_string();
  return fs::PathBuf::from(s.as_str());
}

auto current_dir() -> fs::PathBuf {
  const auto s = sys::getcwd().into_string();
  return fs::PathBuf::from(s.as_str());
}

auto set_current_dir(fs::Path path) -> bool {
  const auto os_path = ffi::OsString::from(path.as_str());
  const auto ret = sys::chdir(os_path.ptr());
  return ret;
}

}  // namespace sfc::env
