#include "sfc/env.h"
#include "sfc/sys/env.h"

namespace sfc::env {

namespace sys_imp = sys::env;
using sys::OsStr;

static constexpr auto PATH_MAX = 1024U;

auto var(Str key) -> String {
  static constexpr auto MAX_ENV = 4096U;
  char buf[MAX_ENV];

  const auto os_key = OsStr::xnew(key);
  const auto os_val = sys_imp::getenv(os_key.ptr(), buf);
  return String::from(Str::from_cstr(os_val));
}

auto set_var(Str key, Str val) -> bool {
  const auto os_key = OsStr::xnew(key);
  const auto os_val = OsStr::xnew(val);
  return sys_imp::setenv(os_key.ptr(), os_val.ptr());
}

auto remove_var(Str key) -> bool {
  const auto os_key = OsStr::xnew(key);
  return sys_imp::unsetenv(os_key.ptr());
}

auto home_dir() -> fs::Path {
  static char buf[PATH_MAX];

  const auto res = sys_imp::home_dir(buf);
  return fs::Path{Str::from_cstr(res)};
}

auto temp_dir() -> fs::Path {
  static char buf[PATH_MAX];

  const auto res = sys_imp::temp_dir(buf);
  return fs::Path{Str::from_cstr(res)};
}

auto current_exe() -> fs::Path {
  static char buf[PATH_MAX];

  const auto res = sys_imp::current_exe(buf);
  return fs::Path{Str::from_cstr(res)};
}

auto current_dir() -> fs::Path {
  static char buf[PATH_MAX];

  const auto res = sys_imp::getcwd(buf);
  return fs::Path{Str::from_cstr(res)};
}

auto set_current_dir(fs::Path path) -> bool {
  const auto os_path = OsStr::xnew(path.as_str());
  return sys_imp::chdir(os_path.ptr());
}

}  // namespace sfc::env
