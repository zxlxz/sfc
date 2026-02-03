#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::os {

static constexpr auto MAX_KEY = 256U;
static constexpr auto MAX_ENV = 1024U;

template <int N>
static inline auto to_os_str(const char src[], wchar_t (&dst)[N]) -> bool {
  const auto ret = ::MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, N);
  return ret >= 0;
}

template <int N>
static inline auto to_u8_str(const wchar_t src[], char (&dst)[N]) -> bool {
  const auto ret = ::WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, N, nullptr, nullptr);
  return ret >= 0;
}

template <int N>
static inline auto getenv(const char* key, char (&buf)[N]) -> const char* {
  wchar_t os_key[MAX_KEY] = {};
  if (!to_os_str(key, os_key)) {
    return nullptr;
  }

  wchar_t os_env[MAX_ENV] = {};
  const auto len = ::GetEnvironmentVariableW(os_key, os_env, MAX_ENV);
  if (len == 0 || len >= MAX_ENV) {
    return nullptr;
  }

  if (!to_u8_str(os_env, buf)) {
    return nullptr;
  }

  return buf;
}

static inline auto setenv(const char* key, const char* val) -> bool {
  wchar_t os_key[MAX_KEY];
  if (!to_os_str(key, os_key)) {
    return false;
  }

  wchar_t os_val[MAX_ENV];
  if (!to_os_str(val, os_val)) {
    return false;
  }

  return ::SetEnvironmentVariableW(os_key, os_val);
}

static inline auto unsetenv(const char* key) -> bool {
  wchar_t os_key[MAX_KEY] = {};
  if (!to_os_str(key, os_key)) {
    return false;
  }

  return ::SetEnvironmentVariableW(os_key, nullptr);
}

static inline auto home_dir() -> PCSTR {
  wchar_t os_buf[MAX_PATH] = {};
  const auto len = ::GetEnvironmentVariableW(L"USERPROFILE", os_buf, MAX_PATH);
  if (len == 0 || len >= MAX_PATH) {
    return nullptr;
  }

  static char u8_buf[MAX_PATH];
  if (!to_u8_str(os_buf, u8_buf)) {
    return nullptr;
  }

  return u8_buf;
}

static inline auto temp_dir() -> PCSTR {
  wchar_t os_path[MAX_PATH] = {};
  const auto len = ::GetTempPath2W(MAX_PATH, os_path);
  if (len <= 0 || len >= MAX_PATH) {
    return nullptr;
  }

  static char u8_path[MAX_PATH];
  if (!to_u8_str(os_path, u8_path)) {
    return nullptr;
  }

  return u8_path;
}

static inline auto current_exe() -> PCSTR {
  wchar_t os_path[MAX_PATH] = {};
  const auto len = ::GetModuleFileNameW(nullptr, os_path, MAX_PATH);
  if (len <= 0 || len >= MAX_PATH) {
    return nullptr;
  }

  static char u8_path[MAX_PATH];
  if (!to_u8_str(os_path, u8_path)) {
    return nullptr;
  }

  return u8_path;
}

static inline auto getcwd() -> const char* {
  wchar_t os_path[MAX_PATH] = {};
  const auto len = ::GetCurrentDirectoryW(MAX_PATH, os_path);
  if (len <= 0 || len >= MAX_PATH) {
    return nullptr;
  }

  static char u8_path[MAX_PATH];
  if (!to_u8_str(os_path, u8_path)) {
    return nullptr;
  }

  return u8_path;
}

static inline auto chdir(const char* path) -> bool {
  wchar_t os_path[MAX_PATH] = {};
  if (!to_os_str(path, os_path)) {
    return false;
  }

  return ::SetCurrentDirectoryW(os_path);
}

}  // namespace sfc::sys::os
#endif
