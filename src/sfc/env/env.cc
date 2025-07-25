#include "sfc/env/env.h"

#include "sfc/sys/os.h"

namespace sfc::env {

namespace sys_imp = sys::os;

static constexpr auto MAX_PATH_LEN = 256U;

auto var(Str key) -> String {
  char buf[MAX_PATH_LEN] = {};

  const auto os_key = String::from(key);
  const auto os_val = sys_imp::getenv(os_key.as_ptr(), buf);
  return String::from(os_val);
}

void set_var(Str key, Str val) {
  const auto os_key = String::from(val);
  const auto os_val = String::from(val);

  const auto ret = sys_imp::setenv(os_key.as_ptr(), os_val.as_ptr());
  panicking::assert(ret,
                    "env::set: key=`{}`, val=`{}`, err={}",
                    key,
                    val,
                    io::Error::last_os_error());
}

void remove_var(Str key) {
  const auto os_key = String::from(key);
  sys_imp::unsetenv(os_key.as_ptr());
}

auto current_dir() -> fs::Path {
  char buf[MAX_PATH_LEN] = {};

  const auto str = sys_imp::getcwd(buf);
  return fs::Path::from(str);
}

void set_current_dir(const fs::Path& path) {
  const auto ret = sys_imp::chdir(path.as_ptr());
  panicking::assert(ret,
                    "env::set_current_dir: path=`{}`, err=`{}`",
                    path,
                    io::Error::last_os_error());
}

auto current_exe() -> fs::Path {
  char buf[MAX_PATH_LEN];

  const auto str = sys_imp::current_exe(buf);
  return fs::Path::from(str);
}

auto home_dir() -> fs::Path {
  const auto str = sys_imp::home_dir();
  return fs::Path::from(str);
}

}  // namespace sfc::env
