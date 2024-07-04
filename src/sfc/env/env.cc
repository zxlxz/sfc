#include "env.h"

#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

namespace sfc::env {

using namespace fs;
using namespace io;
using namespace ffi;

auto get(Str key) -> Str {
  const auto skey = CString::from(key);
  const auto sval = ::getenv(skey);
  if (sval == nullptr) {
    return {};
  }
  return Str{sval};
}

void set(Str key, Str val) {
  if (key.is_empty()) {
    return;
  }

  const auto skey = CString::from(key);
  if (val.is_empty()) {
    (void)::unsetenv(skey);
    return;
  }

  const auto sval = CString::from(val);
  const auto ret = ::setenv(skey, sval, 1);
  assert_fmt(ret != -1, "env::set: key=`{}`, val=`{}`, err={}", key, val,
             io::Error::last_os_error());
}

auto current_dir() -> Path {
  static thread_local char buf[256];

  const auto res = ::getcwd(buf, sizeof(buf));
  assert_fmt(res != nullptr, "env::current_dir: err=`{}`", io::Error::last_os_error());

  return Path{res};
}

void set_current_dir(const Path& path) {
  const auto os_path = CString::from(path.as_str());

  const auto ret = ::chdir(os_path);
  assert_fmt(ret != -1, "env::set_current_dir: path=`{}`, err=`{}`", path,
             io::Error::last_os_error());
}

auto current_exe() -> Path {
  static thread_local char buf[256];

  const auto ret = ::readlink("/proc/self/exe", buf, sizeof(buf));
  assert_fmt(ret != -1, "env::current_exe: err=`{}`", io::Error::last_os_error());

  const auto res = Path{buf};
  return res;
}

auto home_dir() -> Path {
  static const auto uid = getuid();
  static const auto usr = getpwuid(uid);
  static const auto dir = Str{usr->pw_dir};
  return dir;
}

}  // namespace sfc::env
