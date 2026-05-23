
#include "sfc/ffi/wstr.h"

namespace sfc::ffi {

auto wide_codelen(wchar_t h) -> usize {
  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    return 1;
  }

  if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
    return chr::utf16_codelen(u16(h));
  }
}

auto wide_encode(wchar_t (&wbuf)[2], char32_t ch) -> usize {
  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    wbuf[0] = wchar_t(ch);
    return 1;
  }

  if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
    u16 buf[2] = {};
    const auto n = chr::utf16_encode(buf, ch);
    wbuf[0] = wchar_t(buf[0]);
    wbuf[1] = wchar_t(buf[1]);
    return n;
  }
}

auto wide_decode(const wchar_t wbuf[], usize n) -> char32_t {
  if (n == 0) return chr::INVALID;
  if (n == 1) return char32_t(wbuf[0]);

  if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
    if (n == 2) {
      u16 buf[2] = {u16(wbuf[0]), u16(wbuf[1])};
      return chr::utf16_decode(buf, n);
    }
  }
  return chr::INVALID;
}

auto WChars::next() noexcept -> Option<Item> {
  if (_ptr == _end) return {};

  const auto n = ffi::wide_codelen(*_ptr);
  if (n == 0 || _ptr + n > _end) {
    _ptr = _end;
    return {};
  }

  const auto ch = ffi::wide_decode(_ptr, n);
  _ptr += n;
  return ch;
}

auto WStr::chars() const -> WChars {
  return WChars{_ptr, _len};
}

auto WStr::to_string() const -> String {
  if (_len == 0) {
    return {};
  }

  // each wchar_t takes 1-4 u8 bytes
  // reserve 2x here is a balance between memory usage and performance for most cases
  static constexpr auto kRatio = 2U;
  auto res = String::with_capacity(kRatio * _len);
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

  // pop trailing null character
  if (!_buf.is_empty()) {
    _buf.pop();
  }

  _buf.reserve(s.len());
  s.chars().for_each([&](char32_t c) {
    wchar_t buf[2] = {};
    const auto len = ffi::wide_encode(buf, c);
    _buf.extend_from_slice({buf, len});
  });
  _buf.push(0);

  return;
}

// using this name, just make CString and WString have the same API
// even though WString::into_string() is not consuming self.
auto WString::into_string() const -> String {
  return this->as_wstr().to_string();
}

}  // namespace sfc::ffi
