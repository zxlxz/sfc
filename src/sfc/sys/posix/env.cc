#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <pwd.h>
#include <sys/param.h>

#include "sfc/sys/posix/env.h"
#include "sfc/ffi/cstr.h"

namespace sfc::sys::posix {

using ffi::CString;

auto getenv(Str key) -> String {
  if (key.is_empty()) {
    return {};
  }

  const auto c_key = CString::from_str(key);
  const auto c_val = ::getenv(c_key.as_ptr());
  if (c_val == nullptr) {
    return {};
  }

  return String::from(c_val);
}

auto setenv(Str key, Str val) -> bool {
  const auto c_key = CString::from_str(key);
  const auto c_val = CString::from_str(val);
  const auto ret = ::setenv(c_key.as_ptr(), c_val.as_ptr(), 1);
  return ret != -1;
}

auto unsetenv(Str key) -> bool {
  const auto c_key = CString::from_str(key);
  const auto ret = ::unsetenv(c_key.as_ptr());
  return ret != -1;
}

auto home_dir() -> String {
  char buf[4096];
  struct passwd pwd{};
  struct passwd* usr = nullptr;
  const auto ret = ::getpwuid_r(getuid(), &pwd, buf, sizeof(buf), &usr);
  if (ret != 0 || usr == nullptr || usr->pw_dir == nullptr) {
    return {};
  }

  return String::from(usr->pw_dir);
}

auto chdir(Str path) -> bool {
  const auto c_path = CString::from_str(path);
  const auto ret = ::chdir(c_path.as_ptr());
  return ret != -1;
}

auto getcwd() -> String {
  char buf[MAXPATHLEN];
  const auto p = ::getcwd(buf, sizeof(buf));
  if (p == nullptr) {
    return {};
  }

  return String::from(p);
}

}  // namespace sfc::sys::posix
