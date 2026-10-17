
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <pwd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "sfc/sys/posix/env.h"

namespace sfc::sys::posix {

auto getenv(const char* key) -> String {
  const auto c_val = ::getenv(key);
  return String::from(c_val);
}

auto setenv(const char* key, const char* val) -> bool {
  if (val == nullptr) {
    return ::unsetenv(key) != -1;
  }

  return ::setenv(key, val, 1) != -1;
}

auto unsetenv(const char* key) -> bool {
  return ::unsetenv(key) != -1;
}

auto home_dir() -> String {
  const auto uid = getuid();
  const auto usr = getpwuid(uid);
  if (!usr || !usr->pw_dir) {
    return {};
  }

  return String::from(usr->pw_dir);
}

auto temp_dir() -> String {
#ifdef __APPLE__
  char buf[PATH_MAX];
  if (::confstr(_CS_DARWIN_USER_TEMP_DIR, buf, sizeof(buf)) <= 0) {
    return {};
  }
  return String::from(buf);
#else
  return String::from("/tmp");
#endif
}

auto current_exe() -> String {
  char buf[1024];
#ifdef __APPLE__
  auto size = uint32_t{sizeof(buf)};
  if (_NSGetExecutablePath(buf, &size) != 0) {
    return {};
  }
#else
  if (::readlink("/proc/self/exe", buf, sizeof(buf)) == -1) {
    return {};
  }
#endif
  return String::from(buf);
}

auto getcwd() -> String {
  char buf[1024];
  const auto s = ::getcwd(buf, sizeof(buf));
  return String::from(s);
}

auto chdir(const char* path) -> bool {
  return ::chdir(path) != -1;
}

}  // namespace sfc::sys::posix
