#pragma once

#include <windows.h>

#include "sfc/core.h"

namespace sfc::sys::env {

auto getenv(const char* key) -> Str {
  static thread_local char buf[1024];

  const auto num_chars = ::GetEnvironmentVariableA(key, buf, sizeof(buf));
  if (num_chars == 0) {
    return {};
  }

  return Str{buf, num_chars};
}

auto setenv(const char* key, const char* val) -> bool {
  const auto ret = ::SetEnvironmentVariableA(key, val);
  return ret != 0;
}

auto unsetenv(const char* key) -> bool {
  const auto ret = ::SetEnvironmentVariableA(key, nullptr);
  return ret != 0;
}

auto getcwd() -> Str {
  static thread_local char buf[1024];
  
  const auto num_chars = ::GetCurrentDirectoryA(sizeof(buf), buf);
  if (num_chars == 0 || num_chars > sizeof(buf)) {
    return {};
  }

  return Str{buf, num_chars};
}

auto chdir(const char* path) -> bool {
  const auto ret = ::SetCurrentDirectoryA(path);
  return ret != 0;
}

auto current_exe() -> Str {
  static thread_local char buf[1024];

  const auto num_chars = ::GetModuleFileNameA(nullptr, buf, sizeof(buf));
  if (num_chars == 0 || num_chars >= sizeof(buf)) {
    return {};
  }

  return Str{buf, num_chars};
}

auto home_dir() -> Str {
  static thread_local char buf[1024];
  const auto num_chars = ::GetEnvironmentVariableA("USERPROFILE", buf, sizeof(buf));
  if (num_chars == 0 || num_chars > sizeof(buf)) {
    return {};
  }

  return Str{buf, num_chars};
}

}  // namespace sfc::sys::env
