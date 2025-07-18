#pragma once

#include <Windows.h>

namespace sfc::sys::os {

static constexpr auto kMaxPathSize = 1024U;
static constexpr auto kMaxEnvSize = 4096U;

static inline auto getenv(const char* key, char buf[], SIZE_T buf_len) -> const char* {
  const auto num_chars = ::GetEnvironmentVariableA(key, buf, buf_len);
  return num_chars > 0 ? buf : nullptr;
}

static inline auto setenv(const char* key, const char* val) -> bool {
  return ::SetEnvironmentVariableA(key, val);
}

static inline auto unsetenv(const char* key) -> bool {
  return ::SetEnvironmentVariableA(key, nullptr);
}

static inline auto getcwd(char buf[], DWORD buf_len) -> DWORD {
  const auto num_chars = ::GetCurrentDirectoryA(buf_len, buf);
  return num_chars;
}

static inline auto chdir(const char* path) -> bool {
  return ::SetCurrentDirectoryA(path);
}

static inline auto current_exe(char buf[], SIZE_T buf_len) -> SIZE_T {
  const auto res = ::GetModuleFileNameA(nullptr, buf, buf_len);
  return res > 0 ? static_cast<SIZE_T>(res) : 0;
}

static inline auto home_dir() -> const char* {
  static char buf[kMaxPathSize] = {};

  // USERPROFILE is the environment variable that contains the path to the user's home directory
  static const auto num_chars = ::GetEnvironmentVariableA("USERPROFILE", buf, sizeof(buf));
  return num_chars > 0 ? buf : nullptr;
}

}  // namespace sfc::sys::os
