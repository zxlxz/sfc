#include "sfc/env/env.h"

#include "sfc/sys/os.h"

namespace sfc::env {

namespace sys_imp = sys::os;

static constexpr auto MAX_PATH_LEN = 256U;

auto var(Str key) -> String {
  char buf[MAX_PATH_LEN] = {};

  const auto c_key = CString::from(key);
  const auto c_val = Str::from(sys_imp::getenv(c_key, buf));
  return String::from(c_val);
}

void set_var(Str key, Str val) {
  const auto c_key = CString::from(key);
  const auto c_val = CString::from(val);

  const auto ret = sys_imp::setenv(c_key, c_val);
  panicking::assert(ret,
                    "env::set: key=`{}`, val=`{}`, err={}",
                    key,
                    val,
                    io::Error::last_os_error());
}

void remove_var(Str key) {
  const auto c_key = CString::from(key);
  sys_imp::unsetenv(c_key);
}

auto current_dir() -> fs::Path {
  char buf[MAX_PATH_LEN] = {};

  const auto str = Str::from(sys_imp::getcwd(buf));
  return fs::Path::from(str);
}

void set_current_dir(const fs::Path& path) {
  const auto c_path = CString::from(path.as_str());

  const auto ret = sys_imp::chdir(c_path);
  panicking::assert(ret,
                    "env::set_current_dir: path=`{}`, err=`{}`",
                    path,
                    io::Error::last_os_error());
}

auto current_exe() -> fs::Path {
  char buf[MAX_PATH_LEN];

  const auto str = Str::from(sys_imp::current_exe(buf));
  return fs::Path::from(str);
}

auto home_dir() -> fs::Path {
  const auto str = Str::from(sys_imp::home_dir());
  return fs::Path::from(str);
}

}  // namespace sfc::env
