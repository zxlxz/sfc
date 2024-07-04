#include "sfc/env.h"

#include "sfc/sys/os.h"

namespace sfc::env {

namespace sys_imp = sys::os;

static constexpr auto MAX_PATH_LEN = 256U;

auto var(Str key) -> String {
  const auto c_key = CString::from(key);
  const auto c_val = sys_imp::getenv(c_key);
  return String::from(Str::from(c_val));
}

auto set_var(Str key, Str val) -> bool {
  const auto c_key = CString::from(key);
  if (!val) {
    return sys_imp::unsetenv(c_key);
  }

  const auto c_val = CString::from(val);
  return sys_imp::setenv(c_key, c_val);
}

auto remove_var(Str key) -> bool {
  const auto c_key = CString::from(key);
  return sys_imp::unsetenv(c_key);
}

auto current_dir() -> Str {
  const auto res = sys_imp::getcwd();
  return Str::from(res);
}

auto set_current_dir(Str path) -> bool {
  const auto c_path = CString::from(path);
  return sys_imp::chdir(c_path);
}

auto current_exe() -> Str {
  const auto res = sys_imp::current_exe();
  return Str::from(res);
}

auto home_dir() -> Str {
  const auto res = sys_imp::home_dir();
  return Str::from(res);
}

}  // namespace sfc::env
