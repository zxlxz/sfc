
#include "sfc/ffi/wstr.h"

namespace sfc::ffi {

auto encode_wide(Slice<wchar_t> wbuf, char32_t ch) -> WStr {
  if (wbuf._len == 0) {
    return {nullptr, 0};
  }

  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    wbuf._ptr[0] = wchar_t(ch);
    return {wbuf._ptr, 1};
  }

  if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
    auto u16_buf = Slice{ptr::cast<u16>(wbuf._ptr), wbuf._len};
    auto u16_str = chr::encode_utf16(u16_buf, ch);
    return {ptr::cast<wchar_t>(u16_str._ptr), u16_str._len};
  }
}

auto decode_wide(Slice<const wchar_t> wbuf) -> chr::Decode {
  if (wbuf._len == 0) {
    return {chr::INVALID, 0};
  }

  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    return {char32_t(wbuf[0]), 1};
  }

  if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
    auto u16_buf = Slice{ptr::cast<const u16>(wbuf._ptr), wbuf._len};
    return chr::decode_utf16(u16_buf);
  }
}

auto WChars::len() const -> usize {
  return _len;
}

auto WChars::next() noexcept -> Option<char32_t> {
  if (_len == 0) {
    return {};
  }

  const auto [ch, n] = ffi::decode_wide({_ptr, _len});
  _ptr += n;
  _len -= n;
  return ch;
}

auto WStr::chars() const -> WChars {
  return WChars{_ptr, _len};
}

auto WStr::to_string() const -> String {
  if (_len == 0) {
    return {};
  }

  auto res = String::with_capacity(_len);
  auto chars = WChars{_ptr, _len};
  chars.for_each([&](char32_t ch) { res.push(ch); });
  return res;
}

auto WString::from(Str s) -> WString {
  auto res = WString{};
  res.push_str(s);
  return res;
}

auto WString::buf() -> Buf& {
  return _buf;
}

auto WString::len() const -> usize {
  const auto n = _buf.len();
  // exclude the trailing null character if exists
  return n == 0 ? 0 : n - 1;
}

auto WString::as_ptr() const -> const wchar_t* {
  return _buf.as_ptr();
}

auto WString::as_wstr() const -> WStr {
  const auto p = this->as_ptr();
  const auto n = this->len();
  return WStr{p, n};
}

void WString::clear() {
  _buf.clear();
}

void WString::push_str(Str s) {
  if (s.is_empty()) {
    return;
  }

  _buf.reserve(s.len());
  s.chars().for_each([&](char32_t ch) {
    wchar_t buf[2] = {};
    const auto wstr = ffi::encode_wide(buf, ch);
    this->push_wstr(wstr);
  });

  return;
}

void WString::push_wstr(WStr s) {
  if (s._len == 0) {
    return;
  }

  // pop trailing null character
  if (!_buf.is_empty()) {
    _buf.pop();
  }

  _buf.reserve(s._len);
  _buf.extend_from_slice({s._ptr, s._len});
  _buf.push(0);
}

// using this name, just make CString and WString have the same API
// even though WString::into_string() is not consuming self.
auto WString::into_string() const -> String {
  return this->as_wstr().to_string();
}

}  // namespace sfc::ffi
