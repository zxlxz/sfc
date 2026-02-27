#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::env {

static auto getenv(const wchar_t* key) -> ffi::OsString {
  auto f = [&](wchar_t* buf, DWORD buf_len) {
    return ::GetEnvironmentVariableW(key, buf, buf_len);
  };
  return sys::build_string(f);
}

static auto setenv(const wchar_t* key, const wchar_t* val) -> bool {
  return ::SetEnvironmentVariableW(key, val);
}

static auto unsetenv(const wchar_t* key) -> bool {
  return ::SetEnvironmentVariableW(key, nullptr);
}

static auto home_dir() -> ffi::OsString {
  auto f = [&](wchar_t* buf, auto buf_len) {
    return ::GetEnvironmentVariableW(L"USERPROFILE", buf, buf_len);
  };
  return sys::build_string(f);
}

static auto temp_dir() -> ffi::OsString {
  auto f = [](wchar_t* buf, auto buf_len) {
    return ::GetTempPathW(buf_len, buf);
  };
  return sys::build_string(f);
}

static auto current_exe() -> ffi::OsString {
  auto f = [](wchar_t* buf, DWORD buf_len) {
    return ::GetModuleFileNameW(nullptr, buf, buf_len);
  };
  return sys::build_string(f, MAX_PATH);
}

static auto getcwd() -> ffi::OsString {
  auto f = [](wchar_t* buf, auto buf_len) {
    return ::GetCurrentDirectoryW(buf_len, buf);
  };
  return sys::build_string(f);
}

static auto chdir(const wchar_t* path) -> bool {
  return ::SetCurrentDirectoryW(path);
}

}  // namespace sfc::sys::env
