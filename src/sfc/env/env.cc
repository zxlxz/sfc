#include "env.h"

#include "sfc/sys/env.inl"

namespace sfc::env {

namespace sys_imp = sys::env;

auto var(Str key) -> Str {
  const auto c_key = String::from(key);
  return sys_imp::getenv(c_key.c_str());
}

void set_var(Str key, Str val) {
  const auto c_key = String::from(val);
  const auto c_val = String::from(val);

  const auto ret = sys_imp::setenv(c_key.c_str(), c_val.c_str());
  assert_fmt(ret, "env::set: key=`{}`, val=`{}`, err={}", key, val, io::Error::last_os_error());
}

void remove_var(Str key) {
  const auto c_key = String::from(key);
  sys_imp::unsetenv(c_key.c_str());
}

auto current_dir() -> fs::Path {
  const auto res = sys_imp::getcwd();
  return fs::Path::from(res);
}

void set_current_dir(const fs::Path& path) {
  const auto ret = sys_imp::chdir(path.c_str());
  assert_fmt(ret, "env::set_current_dir: path=`{}`, err=`{}`", path, io::Error::last_os_error());
}

auto current_exe() -> fs::Path {
  const auto res = sys_imp::current_exe();
  return fs::Path::from(res);
}

auto home_dir() -> fs::Path {
  const auto res = sys_imp::home_dir();
  return fs::Path::from(res);
}

}  // namespace sfc::env
