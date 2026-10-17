#include <Windows.h>
#include "sfc/sys/windows/env.h"

namespace sfc::sys::windows {

static constexpr u32 BUF_SIZE = 1024U;

static auto make_string(const wchar_t* buf, u32 len) -> String {
  const auto wstr = Slice{(const u16*)buf, len};
  return String::from_utf16(wstr);
}

auto getenv(const wchar_t* key) -> String {
  wchar_t buf[BUF_SIZE];
  const auto len = ::GetEnvironmentVariableW(key, buf, BUF_SIZE);
  return make_string(buf, len);
}

auto setenv(const wchar_t* key, const wchar_t* val) -> bool {
  const auto ret = ::SetEnvironmentVariableW(key, val);
  return bool(ret);
}

auto unsetenv(const wchar_t* key) -> bool {
  const auto ret = ::SetEnvironmentVariableW(key, nullptr);
  return bool(ret);
}

auto home_dir() -> String {
  wchar_t buf[BUF_SIZE];
  const auto len = ::GetEnvironmentVariableW(L"USERPROFILE", buf, BUF_SIZE);
  return make_string(buf, len);
}

auto temp_dir() -> String {
  wchar_t buf[BUF_SIZE];
  const auto len = ::GetTempPathW(BUF_SIZE, buf);
  return make_string(buf, len);
}

auto current_exe() -> String {
  wchar_t buf[BUF_SIZE];
  const auto len = ::GetModuleFileNameW(nullptr, buf, BUF_SIZE);
  return make_string(buf, len);
}

auto getcwd() -> String {
  wchar_t buf[BUF_SIZE];
  const auto len = ::GetCurrentDirectoryW(BUF_SIZE, buf);
  return make_string(buf, len);
}

auto chdir(const wchar_t* path) -> bool {
  const auto ret = ::SetCurrentDirectoryW(path);
  return bool(ret);
}

}  // namespace sfc::sys::windows
