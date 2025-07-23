#include "sfc/env/env.h"

#include "sfc/ffi/cstring.h"
#include "sfc/sys/os.h"

namespace sfc::env {

namespace sys_imp = sys::os;

static constexpr auto MAX_PATH_LEN = 256U;

auto var(Str key) -> String {
  char buf[MAX_PATH_LEN] = {};

  const auto os_key = ffi::CString::from(key);
  const auto os_val = sys_imp::getenv(os_key.c_str(), buf);
  return String::from(Str::from_cstr(os_val));
}

void set_var(Str key, Str val) {
  const auto os_key = ffi::CString::from(val);
  const auto os_val = ffi::CString::from(val);

  const auto ret = sys_imp::setenv(os_key.c_str(), os_val.c_str());
  panicking::assert(ret,
                    "env::set: key=`{}`, val=`{}`, err={}",
                    key,
                    val,
                    io::Error::last_os_error());
}

void remove_var(Str key) {
  const auto os_key = ffi::CString::from(key);
  sys_imp::unsetenv(os_key.c_str());
}

auto current_dir() -> fs::Path {
  char buf[MAX_PATH_LEN] = {};

  const auto str = sys_imp::getcwd(buf);
  return fs::Path::from(Str::from_cstr(str));
}

void set_current_dir(const fs::Path& path) {
  const auto os_path = ffi::CString::from(path.as_str());

  const auto ret = sys_imp::chdir(os_path.c_str());
  panicking::assert(ret,
                    "env::set_current_dir: path=`{}`, err=`{}`",
                    path,
                    io::Error::last_os_error());
}

auto current_exe() -> fs::Path {
  char buf[MAX_PATH_LEN];

  const auto s = sys_imp::current_exe(buf);
  auto p = fs::Path::from(Str::from_cstr(s));
  return p;
}

auto home_dir() -> fs::Path {
  const auto s = sys_imp::home_dir();
  auto p = fs::Path::from(Str::from_cstr(s));
  return p;
}

}  // namespace sfc::env
