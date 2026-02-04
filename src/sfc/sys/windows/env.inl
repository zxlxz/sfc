#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::env {

template <int N>
static auto getenv(const wchar_t* key, char (&buf)[N]) -> const char* {
  wchar_t os_buf[N];
  const auto len = ::GetEnvironmentVariableW(key, os_buf, N);
  if (len <= 0 || len >= N) {
    return nullptr;
  }
  return to_utf8(os_buf, buf);
}

static auto setenv(const wchar_t* key, const wchar_t* val) -> bool {
  return ::SetEnvironmentVariableW(key, val);
}

static auto unsetenv(const wchar_t* key) -> bool {
  return ::SetEnvironmentVariableW(key, nullptr);
}

template <int BUF_SIZE>
static auto home_dir(char (&buf)[BUF_SIZE]) -> char* {
  wchar_t os_path[BUF_SIZE];
  const auto len = ::GetEnvironmentVariableW(L"USERPROFILE", os_path, BUF_SIZE);
  if (len == 0 || len >= BUF_SIZE) {
    return nullptr;
  }

  return to_utf8(os_path, buf);
}

template <int BUF_SIZE>
static auto temp_dir(char (&buf)[BUF_SIZE]) -> char* {
  wchar_t os_path[BUF_SIZE];
  const auto len = ::GetTempPath2W(BUF_SIZE, os_path);
  if (len == 0 || len >= BUF_SIZE) {
    return nullptr;
  }

  return to_utf8(os_path, buf);
}

template <int BUF_SIZE>
static auto current_exe(char (&buf)[BUF_SIZE]) -> char* {
  wchar_t os_path[BUF_SIZE];
  const auto len = ::GetModuleFileNameW(nullptr, os_path, BUF_SIZE);
  if (len == 0 || len >= BUF_SIZE) {
    return nullptr;
  }
  return to_utf8(os_path, buf);
}

template <int BUF_SIZE>
static auto getcwd(char (&buf)[BUF_SIZE]) -> char* {
  wchar_t os_path[BUF_SIZE];
  const auto len = ::GetCurrentDirectoryW(BUF_SIZE, os_path);
  if (len == 0 || len >= BUF_SIZE) {
    return nullptr;
  }
  return to_utf8(os_path, buf);
}

static auto chdir(const wchar_t* path) -> bool {
  return ::SetCurrentDirectoryW(path);
}

}  // namespace sfc::sys::env
