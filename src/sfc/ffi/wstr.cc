
#include "sfc/ffi/wstr.h"

namespace sfc::ffi {

auto wstrlen(const wchar_t* s) -> usize {
  if (s == nullptr) {
    return 0;
  }

  auto n = usize{0};
  while (s[n] != 0) {
    ++n;
  }
  return n;
}

auto WStr::to_string() const -> String {
  if (_len == 0) {
    return String{};
  }

  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    auto res = String{};
    for (auto ch : Slice{_ptr, _len}) {
      res.push(char32_t(ch));
    }
    return res;
  }

  if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
    const auto u16_buf = Slice{(const u16*)_ptr, _len};
    return String::from_utf16(u16_buf);
  }
}

auto WCStr::to_string() const -> String {
  const auto len = wstrlen(_inn);
  const auto str = WStr{_inn, len};
  return str.to_string();
}

auto WCString::from_str(Str s) -> WCString {
  if (s.is_empty()) {
    return {};
  }

  auto wcstr = WCString{};
  wcstr._buf.reserve(s.len());
  s.chars().for_each([&](char32_t ch) { wcstr.push(ch); });
  return wcstr;
}

auto WCString::as_ptr() const -> const wchar_t* {
  return _buf.as_ptr();
}

void WCString::push(char32_t c) {
  if (!_buf.is_empty()) {
    _buf.pop();
  }

  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    _buf.push(wchar_t(c));
  }

  if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
    u16 buf[2] = {};
    const auto u16_str = chr::encode_utf16(buf, c);
    const auto u16_len = u16_str._len;
    _buf.extend_from_slice({(wchar_t*)buf, u16_len});
  }

  _buf.push(0);
}

void WCString::push_str(Str s) {
  if (s.is_empty()) {
    return;
  }

  _buf.reserve(s.len());
  s.chars().for_each([&](char32_t ch) { this->push(ch); });

  return;
}

auto WCString::into_string() const -> String {
  const auto ws = WCStr{_buf.as_ptr()};
  return ws.to_string();
}

}  // namespace sfc::ffi
