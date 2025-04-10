#pragma once

#include <windows.h>

namespace sfc::sys::os {

static constexpr auto kMaxPathSize = 1024U;
static constexpr auto kMaxEnvSize = 4096U;

static inline auto getenv(const char* key) -> const char* {
  static thread_local char buf[kMaxEnvSize];

  const auto num_chars = ::GetEnvironmentVariableA(key, buf, sizeof(buf));
  return num_chars > 0 ? buf : nullptr;
}

static inline auto setenv(const char* key, const char* val) -> bool {
  return ::SetEnvironmentVariableA(key, val);
}

static inline auto unsetenv(const char* key) -> bool {
  return ::SetEnvironmentVariableA(key, nullptr);
}

static inline auto getcwd() -> const char* {
  static thread_local char buf[kMaxPathSize];

  const auto num_chars = ::GetCurrentDirectoryA(sizeof(buf), buf);
  return num_chars > 0 ? buf : nullptr;
}

static inline auto chdir(const char* path) -> bool {
  return ::SetCurrentDirectoryA(path);
}

static inline auto current_exe() -> const char* {
  static thread_local char buf[kMaxPathSize];

  const auto num_chars = ::GetModuleFileNameA(nullptr, buf, sizeof(buf));
  return num_chars > 0 ? buf : nullptr;
}

static inline auto home_dir() -> const char* {
  static thread_local char buf[kMaxPathSize];

  // USERPROFILE is the environment variable that contains the path to the user's home directory
  const auto num_chars = ::GetEnvironmentVariableA("USERPROFILE", buf, sizeof(buf));
  return num_chars > 0 ? buf : nullptr;
}

}  // namespace sfc::sys::os
