#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::os {

static constexpr auto kMaxEnvSize = 4096U;
static constexpr auto kMaxPathSize = 256U;

static inline auto getenv(const char* key) -> const char* {
  static char buf[kMaxEnvSize] = {};
  const auto len = ::GetEnvironmentVariableA(key, buf, sizeof(buf));
  if (len == 0 || len >= sizeof(buf)) {
    return nullptr;
  }
  return buf;
}

static inline auto setenv(const char* key, const char* val) -> bool {
  return ::SetEnvironmentVariableA(key, val) != 0;
}

static inline auto unsetenv(const char* key) -> bool {
  return ::SetEnvironmentVariableA(key, nullptr) != 0;
}

static inline auto home_dir() -> PCSTR {
  static char buf[kMaxPathSize] = {};
  static const auto len = ::GetEnvironmentVariableA("USERPROFILE", buf, sizeof(buf));
  return len ? buf : nullptr;
}

static inline auto temp_dir() -> PCSTR {
  static char buf[kMaxPathSize] = {};
  static const auto len = ::GetTempPath2A(sizeof(buf), buf);
  return len ? buf : nullptr;
}

static inline auto current_exe() -> PCSTR {
  static char buf[kMaxPathSize] = {};
  static const auto len = ::GetModuleFileNameA(nullptr, buf, sizeof(buf));
  return len ? buf : nullptr;
}

static inline auto getcwd() -> const char* {
  static char buf[kMaxPathSize] = {};
  const auto len = ::GetCurrentDirectoryA(sizeof(buf), buf);
  return len ? buf : nullptr;
}

static inline auto chdir(const char* path) -> bool {
  return ::SetCurrentDirectoryA(path) != 0;
}

}  // namespace sfc::sys::os
#endif
