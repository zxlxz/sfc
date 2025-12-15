#include "sfc/env.h"

#include "sfc/sys/os.h"
#include "sfc/ffi/c_str.h"

namespace sfc::env {

namespace sys_imp = sys::os;
using ffi::CString;

auto var(Str key) -> String {
  const auto c_key = CString::xnew(key);
  const auto c_val = sys_imp::getenv(c_key.as_ptr());
  return String::from(Str{c_val});
}

auto set_var(Str key, Str val) -> bool {
  const auto c_key = CString::xnew(key);
  if (val.is_empty()) {
    return sys_imp::unsetenv(c_key.as_ptr());
  }

  const auto c_val = CString::xnew(val);
  return sys_imp::setenv(c_key.as_ptr(), c_val.as_ptr());
}

auto remove_var(Str key) -> bool {
  const auto c_key = CString::xnew(key);
  return sys_imp::unsetenv(c_key.as_ptr());
}

auto home_dir() -> fs::Path {
  const auto res = sys_imp::home_dir();
  return fs::Path{Str{res}};
}

auto temp_dir() -> fs::Path {
  const auto res = sys_imp::temp_dir();
  return fs::Path{Str{res}};
}

auto current_exe() -> fs::Path {
  const auto res = sys_imp::current_exe();
  return fs::Path{Str{res}};
}

auto current_dir() -> fs::Path {
  const auto res = sys_imp::getcwd();
  return fs::Path{Str{res}};
}

auto set_current_dir(const fs::Path& path) -> bool {
  const auto c_path = CString::xnew(path.as_str());
  return sys_imp::chdir(c_path.as_ptr());
}

}  // namespace sfc::env
