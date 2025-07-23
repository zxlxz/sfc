#pragma once

#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

namespace sfc::sys::os {

template <size_t N>
static auto getenv(const char* key, char (&buf)[N]) -> const char* {
  (void)buf;
  const auto res = ::getenv(key);
  return res;
}
static auto setenv(const char* key, const char* val) -> bool {
  if (val == nullptr) {
    const auto ret = ::unsetenv(key);
    return ret == 0;
  }

  const auto ret = ::setenv(key, val, 1);
  return ret == 0;
}

auto unsetenv(const char* key) -> bool {
  const auto ret = ::unsetenv(key);
  return ret == 0;
}

template <size_t N>
auto getcwd(char (&buf)[N]) -> const char* {
  const auto res = ::getcwd(buf, N);
  if (res == nullptr) {
    return nullptr;
  }

  return buf;
}

auto chdir(cstr_t path) -> bool {
  const auto ret = ::chdir(path);
  return ret == 0;
}

template <unsigned short N>
auto current_exe(char (&buf)[N]) -> const char* {
  const auto ret = ::readlink("/proc/self/exe", buf, N);
  if (ret < 0 || ret >= N) {
    return nullptr;
  }
  return buf;
}

auto home_dir() -> const char* {
  const auto uid = getuid();
  const auto usr = getpwuid(uid);
  return usr->pw_dir;
}

}  // namespace sfc::sys::os
