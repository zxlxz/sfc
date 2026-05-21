
#include "sfc/ffi/os_str.h"

namespace sfc::ffi {

auto WStr::chars() const -> chr::WChars {
  return chr::WChars{_ptr, _len};
}

auto WStr::to_string() const -> String {
  if (_len == 0) {
    return {};
  }

  // each wchar_t takes 1-4 u8 bytes
  // reserve 2x here is a balance between memory usage and performance for most cases
  static constexpr auto kRatio = 2U;
  auto res = String::with_capacity(kRatio * _len);
  auto chars = chr::WChars{_ptr, _len};
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
    const auto len = chr::wide_encode(buf, c);
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
