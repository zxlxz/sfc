#pragma once

#include <Windows.h>

namespace sfc::sys::os {

static constexpr auto kMaxEnvSize = 4096U;

template <DWORD N>
static inline auto getenv(const char* key, char (&buf)[N]) -> const char* {
  const auto len = ::GetEnvironmentVariableA(key, buf, N);
  if (len == 0 || len >= N) {
    return nullptr;
  }
  return buf;
}

static inline auto setenv(const char* key, const char* val) -> bool {
  return ::SetEnvironmentVariableA(key, val);
}

static inline auto unsetenv(const char* key) -> bool {
  return ::SetEnvironmentVariableA(key, nullptr);
}

template <DWORD N>
static inline auto getcwd(char (&buf)[N]) -> const char* {
  const auto len = ::GetCurrentDirectoryA(N, buf);
  if (len == 0 || len >= N) {
    return nullptr;
  }
  return buf;
}

static inline auto chdir(const char* path) -> bool {
  const auto ret = ::SetCurrentDirectoryA(path);
  return ret;
}

template <DWORD N>
static inline auto current_exe(char (&buf)[N]) -> PCSTR {
  const auto len = ::GetModuleFileNameA(nullptr, buf, N);
  if (len == 0 || len >= N) {
    return nullptr;
  }
  return buf;
}

static inline auto home_dir() -> PCSTR {
  static char buf[256] = {};
  const auto len = ::GetEnvironmentVariableA("USERPROFILE", buf, sizeof(buf));
  if (len == 0 || len >= sizeof(buf)) {
    return nullptr;
  }
  return buf;
}

}  // namespace sfc::sys::os
