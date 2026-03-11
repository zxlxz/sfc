#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

static auto getenv(const wchar_t* key) -> ffi::WString {
  auto f = [&](wchar_t* buf, DWORD buf_len) {
    return ::GetEnvironmentVariableW(key, buf, buf_len);
  };
  return sys::build_wstring(f);
}

static auto setenv(const wchar_t* key, const wchar_t* val) -> bool {
  const auto ret = ::SetEnvironmentVariableW(key, val);
  return bool(ret);
}

static auto unsetenv(const wchar_t* key) -> bool {
  const auto ret = ::SetEnvironmentVariableW(key, nullptr);
  return bool(ret);
}

static auto home_dir() -> ffi::WString {
  auto f = [&](wchar_t* buf, auto buf_len) {
    return ::GetEnvironmentVariableW(L"USERPROFILE", buf, buf_len);
  };
  return sys::build_wstring(f);
}

static auto temp_dir() -> ffi::WString {
  auto f = [](wchar_t* buf, auto buf_len) {
    return ::GetTempPathW(buf_len, buf);
  };
  return sys::build_wstring(f);
}

static auto current_exe() -> ffi::WString {
  auto f = [](wchar_t* buf, DWORD buf_len) {
    return ::GetModuleFileNameW(nullptr, buf, buf_len);
  };
  return sys::build_wstring(f, MAX_PATH);
}

static auto getcwd() -> ffi::WString {
  auto f = [](wchar_t* buf, auto buf_len) {
    return ::GetCurrentDirectoryW(buf_len, buf);
  };
  return sys::build_wstring(f);
}

static auto chdir(const wchar_t* path) -> bool {
  const auto ret = ::SetCurrentDirectoryW(path);
  return bool(ret);
}

}  // namespace sfc::sys::windows
