#pragma once

// windows.h
#include <Windows.h>

// other system headers
#include <DbgHelp.h>
#include <process.h>
#include <timeapi.h>
#include <winerror.h>

#pragma comment(lib, "DbgHelp.lib")

namespace sfc::sys {

class OsStr {
  wchar_t* _ptr = nullptr;

 public:
  OsStr() noexcept = default;

  OsStr(OsStr&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  ~OsStr() {
    if (_ptr) {
      ::LocalFree(_ptr);
    }
  }

  auto ptr() const {
    return _ptr;
  }

  static auto xnew(auto src) -> OsStr {
    const auto u8_ptr = src._ptr;
    const auto u8_len = static_cast<int>(src._len);
    if (u8_len == 0) {
      return {};
    }

    const auto ws_len = ::MultiByteToWideChar(CP_UTF8, 0, u8_ptr, u8_len, nullptr, 0);
    if (ws_len <= 0) {
      return {};
    }

    const auto ws_cap = ws_len + 1;
    const auto ws_buf = static_cast<wchar_t*>(::LocalAlloc(0, ws_cap * sizeof(wchar_t)));
    ::MultiByteToWideChar(CP_UTF8, 0, u8_ptr, u8_len, ws_buf, ws_cap);
    ws_buf[ws_len] = wchar_t(0);

    auto res = OsStr{};
    res._ptr = ws_buf;

    return res;
  }
};

template <int BUF_SIZE>
static inline auto to_utf8(const wchar_t* wstr, char (&buf)[BUF_SIZE]) -> char* {
  const auto u8_cap = ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buf, BUF_SIZE, nullptr, nullptr);
  if (u8_cap <= 0 || u8_cap >= BUF_SIZE) {
    return nullptr;
  }
  return buf;
}

}  // namespace sfc::sys
