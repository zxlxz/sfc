#pragma once

#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#include "sfc/core.h"

namespace sfc::sys::env {

auto getenv(cstr_t key) -> Str {
  const auto res = ::getenv(key);
  return Str{res};
}

auto setenv(cstr_t key, cstr_t val) -> bool {
  const auto ret = ::setenv(key, val, 1);
  return ret == 0;
}

auto unsetenv(cstr_t key) -> bool {
  const auto ret = ::unsetenv(key);
  return ret == 0;
}

auto getcwd() -> Str {
  static thread_local char buf[256];
  const auto res = ::getcwd(buf, sizeof(buf));
  if (res == nullptr) {
    return {};
  }

  const auto path = Str{buf};
  return path;
}

auto chdir(cstr_t path) -> bool {
  const auto ret = ::chdir(path);
  return ret == 0;
}

auto current_exe() -> Str {
  static thread_local char buf[256];

  const auto ret = ::readlink("/proc/self/exe", buf, sizeof(buf));
  assert_fmt(ret != -1, "env::current_exe: err=`{}`", io::Error::last_os_error());

  const auto path = Str{buf};
  return path;
}

auto home_dir() -> Str {
  const auto uid = getuid();
  const auto usr = getpwuid(uid);

  const auto path = Str{usr->pw_dir};
  return path;
}

}  // namespace sfc::sys::env
