#include <Windows.h>

#include "sfc/sys/windows/io.h"
#include "sfc/sys/windows/env.h"
#include "sfc/sys/windows/wstr.h"

namespace sfc::sys::windows {

constexpr u32 kMaxKey = 256U;
constexpr u32 kMaxVal = 4096U;

auto getenv(Str key) -> String {
  const auto os_key = FixedWStr<kMaxKey>::from_str(key);
  if (!os_key) {
    return String::new_();
  }

  wchar_t val_buf[kMaxVal];
  const auto val_len = ::GetEnvironmentVariableW(os_key, val_buf, kMaxVal);
  return String::from_wstr({val_buf, val_len});
}

auto setenv(Str key, Str val) -> bool {
  const auto os_key = FixedWStr<kMaxKey>::from_str(key);
  const auto os_val = FixedWStr<kMaxVal>::from_str(val);
  if (!os_key || val._len >= kMaxVal - 1) {
    return false;
  }

  const auto ret = ::SetEnvironmentVariableW(os_key, os_val);
  return bool(ret);
}

auto unsetenv(Str key) -> bool {
  const auto os_key = FixedWStr<kMaxKey>::from_str(key);
  if (!os_key) {
    return false;
  }

  const auto ret = ::SetEnvironmentVariableW(os_key, nullptr);
  return bool(ret);
}

auto home_dir() -> String {
  wchar_t path_buf[kMaxPath];
  const auto path_len = ::GetEnvironmentVariableW(L"USERPROFILE", path_buf, kMaxPath);
  return String::from_wstr({path_buf, path_len});
}

auto getcwd() -> String {
  wchar_t path_buf[kMaxPath];

  const auto path_len = ::GetCurrentDirectoryW(kMaxPath, path_buf);
  return String::from_wstr({path_buf, path_len});
}

auto chdir(Str path) -> bool {
  const auto os_path = FixedWStr<kMaxPath>::from_str(path);
  if (!os_path) {
    return false;
  }

  return ::SetCurrentDirectoryW(os_path);
}

}  // namespace sfc::sys::windows
