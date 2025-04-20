#pragma once

#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

namespace sfc::sys::os {

static auto getenv(const char* key, char buf[], size_t buf_len) -> const char* {
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

auto getcwd(char* buf, size_t buf_len) -> size_t {
  const auto res = ::getcwd(buf, buf_len);
  if (res == nullptr) {
    return 0;
  }

  const auto len = __builtin_strlen(buf);
  return len;
}

auto chdir(cstr_t path) -> bool {
  const auto ret = ::chdir(path);
  return ret == 0;
}

auto current_exe(char buf[], size_t buf_len) -> size_t {
  const auto ret = ::readlink("/proc/self/exe", buf, buf_len);
  return ret > 0 ? static_cast<usize>(ret) : 0;
}

auto home_dir() -> const char* {
  const auto uid = getuid();
  const auto usr = getpwuid(uid);
  return usr->pw_dir;
}

}  // namespace sfc::sys::os
