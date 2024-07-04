#pragma once
#if defined(__unix__) || defined(__APPLE__)

#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace sfc::sys::os {

static constexpr auto kMaxPathSize = 256U;

static auto getenv(const char* key) -> const char* {
  return ::getenv(key);
}

static auto setenv(const char* key, const char* val) -> bool {
  if (val == nullptr) {
    return ::unsetenv(key) == 0;
  }

  return ::setenv(key, val, 1) == 0;
}

auto unsetenv(const char* key) -> bool {
  return ::unsetenv(key) == 0;
}

auto home_dir() -> const char* {
  const auto uid = getuid();
  const auto usr = getpwuid(uid);
  return usr->pw_dir;
}

auto temp_dir() -> const char* {
#ifdef __APPLE__
  static char buf[kMaxPathSize] = {};
  static const auto len = ::confstr(_CS_DARWIN_USER_TEMP_DIR, buf, sizeof(buf));
  if (len > 0 && len < sizeof(buf)) {
    return buf;
  }
  return "/tmp";
#else
  return "/tmp";
#endif
}

auto current_exe() -> const char* {
  static char buf[kMaxPathSize] = {};
#ifdef __APPLE__
  auto size = uint32_t{sizeof(buf)};
  if (_NSGetExecutablePath(buf, &size) != 0) {
    return nullptr;
  }
#else
  const auto len = ::readlink("/proc/self/exe", buf, sizeof(buf));
  if (len == -1 || static_cast<size_t>(len) == sizeof(buf)) {
    return nullptr;
  }
  buf[len] = '\0';
#endif
  return buf;
}

auto getcwd() -> const char* {
  static char buf[kMaxPathSize] = {};
  return ::getcwd(buf, sizeof(buf));
}

auto chdir(cstr_t path) -> bool {
  return ::chdir(path) == 0;
}

}  // namespace sfc::sys::os
#endif
