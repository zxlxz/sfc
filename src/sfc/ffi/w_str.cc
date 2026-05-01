
#include "sfc/ffi/w_str.h"

namespace sfc::ffi {

static auto wide_codelen(wchar_t h) -> u32 {
  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    return 1;
  } else {
    if (h < 0xD800) {
      return 1;
    }
    if (h < 0xDBFF) {
      return 2;
    }
    return 0;
  }
}

auto WChars::next() noexcept -> Option<char32_t> {
  if (_ptr >= _end) {
    return {};
  }

  const auto n = wide_codelen(*_ptr);
  if (_ptr + n > _end) {
    _ptr = _end;
    return 0xFFFD;
  }

  switch (n) {
    case 1: {
      const auto a = *_ptr++;
      return a;
    }
    case 2: {
      const auto a = *_ptr++;
      const auto b = *_ptr++;
      return 0x10000 + ((a - 0xD800) << 10) + (b - 0xDC00);
    }
    default: {
      _ptr += 1;
      return 0xFFFD;
    }
  }
}

auto WString::from(Str s) -> WString {
  if (s.is_empty()) {
    return {};
  }

  auto res = WString{};
  auto& buf = res._buf;
  s.chars().for_each([&](char32_t ch) {
    if (ch <= 0xFFFF) {
      buf.push(static_cast<wchar_t>(ch));
    } else {
      const auto t = ch - 0x10000;
      buf.push(static_cast<wchar_t>(0xD800 + (t >> 10)));
      buf.push(static_cast<wchar_t>(0xDC00 + (t & 0x3FF)));
    }
  });
  buf.push(0);

  return res;
}

auto WString::ptr() const -> const wchar_t* {
  return _buf.as_ptr();
}

auto WString::chars() const -> WChars {
  const auto v = _buf.as_slice();
  return WChars{v._ptr, v._ptr + v._len - 1};
}

auto WString::into_string() const -> String {
  if (_buf.len() <= 1) {
    return {};
  }

  auto res = String::with_capacity(_buf.len());
  auto chars = this->chars();
  while (auto ch = chars.next()) {
    res.push(*ch);
  }
  return res;
}

}  // namespace sfc::ffi
