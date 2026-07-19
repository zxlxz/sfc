#include <Windows.h>
#undef min
#undef max

#include "sfc/sys/windows/env.h"

namespace sfc::sys::windows {

static auto build_wstring(auto&& f, unsigned capacity = 0) -> ffi::WString {
  if (capacity == 0) {
    capacity = f(nullptr, 0);
  }

  if (capacity == 0) {
    return {};
  }

  auto res = ffi::WString{};
  auto& buf = res.buf();
  buf.reserve(capacity + 1);
  auto len = f(buf.as_mut_ptr(), capacity);
  buf.set_len(len + 1);
  return res;
}

auto getenv(const wchar_t* key) -> ffi::WString {
  auto f = [&](wchar_t* buf, DWORD buf_len) {
    return ::GetEnvironmentVariableW(key, buf, buf_len);
  };
  return build_wstring(f);
}

auto setenv(const wchar_t* key, const wchar_t* val) -> bool {
  const auto ret = ::SetEnvironmentVariableW(key, val);
  return bool(ret);
}

auto unsetenv(const wchar_t* key) -> bool {
  const auto ret = ::SetEnvironmentVariableW(key, nullptr);
  return bool(ret);
}

auto home_dir() -> ffi::WString {
  auto f = [&](wchar_t* buf, DWORD buf_len) {
    return ::GetEnvironmentVariableW(L"USERPROFILE", buf, buf_len);
  };
  return build_wstring(f);
}

auto temp_dir() -> ffi::WString {
  auto f = [](wchar_t* buf, DWORD buf_len) { return ::GetTempPathW(buf_len, buf); };
  return build_wstring(f);
}

auto current_exe() -> ffi::WString {
  auto f = [](wchar_t* buf, DWORD buf_len) { return ::GetModuleFileNameW(nullptr, buf, buf_len); };
  return build_wstring(f, MAX_PATH);
}

auto getcwd() -> ffi::WString {
  auto f = [](wchar_t* buf, DWORD buf_len) { return ::GetCurrentDirectoryW(buf_len, buf); };
  return build_wstring(f);
}

auto chdir(const wchar_t* path) -> bool {
  const auto ret = ::SetCurrentDirectoryW(path);
  return bool(ret);
}

}  // namespace sfc::sys::windows
