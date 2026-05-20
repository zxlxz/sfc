#include "sfc/core/chr.h"

namespace sfc::chr {

static auto utf8_is_continuation(u8 b) -> bool {
  return (b & 0xC0) == 0x80;
}

auto utf8_codelen(u8 h) -> usize {
  static constexpr auto H0 = 0x00;  // 0b00000000
  static constexpr auto H1 = 0x80;  // 0b10000000
  static constexpr auto H2 = 0xC0;  // 0b11000000
  static constexpr auto H3 = 0xE0;  // 0b11100000
  static constexpr auto H4 = 0xF0;  // 0b11110000
  static constexpr auto H5 = 0xF8;  // 0b11111000

  if ((h & H1) == H0) return 1;
  if ((h & H3) == H2) return 2;
  if ((h & H4) == H3) return 3;
  if ((h & H5) == H4) return 4;
  return 0;
}

auto utf8_encode(char32_t ch, u8 (&buf)[4]) -> usize {
  if (ch > 0x10FFFF) {
    return 0;
  }

  if (ch <= 0x7F) {
    buf[0] = static_cast<u8>(ch);
    return 1;
  }

  if (ch <= 0x7FF) {
    buf[0] = static_cast<u8>(0xC0 | (ch >> 6));
    buf[1] = static_cast<u8>(0x80 | (ch & 0x3F));
    return 2;
  }

  if (ch <= 0xFFFF) {
    buf[0] = static_cast<u8>(0xE0 | (ch >> 12));
    buf[1] = static_cast<u8>(0x80 | ((ch >> 6) & 0x3F));
    buf[2] = static_cast<u8>(0x80 | (ch & 0x3F));
    return 3;
  }

  if (ch <= 0x10FFFF) {
    buf[0] = static_cast<u8>(0xF0 | (ch >> 18));
    buf[1] = static_cast<u8>(0x80 | ((ch >> 12) & 0x3F));
    buf[2] = static_cast<u8>(0x80 | ((ch >> 6) & 0x3F));
    buf[3] = static_cast<u8>(0x80 | (ch & 0x3F));
    return 4;
  }

  return 0;
}

auto utf8_decode(const u8 p[], usize n) -> char32_t {
  switch (n) {
    case 1: {
      const auto a = p[0];
      return static_cast<char32_t>(a);
    }
    case 2: {
      const auto a = p[0] & 0x1F;
      const auto b = p[1] & 0x3F;
      return static_cast<char32_t>((a << 6) | b);
    }
    case 3: {
      const auto a = p[0] & 0x0F;
      const auto b = p[1] & 0x3F;
      const auto c = p[2] & 0x3F;
      return static_cast<char32_t>((a << 12) | (b << 6) | c);
    }
    case 4: {
      const auto a = p[0] & 0x07;
      const auto b = p[1] & 0x3F;
      const auto c = p[2] & 0x3F;
      const auto d = p[3] & 0x3F;
      return static_cast<char32_t>((a << 18) | (b << 12) | (c << 6) | d);
    }
  }
  return chr::INVALID;
}

auto utf16_codelen(u16 h) -> usize {
  if (h < 0xD800) return 1;
  if (h < 0xDBFF) return 2;
  if (h < 0xE000) return 0;
  return 1;
}

auto utf16_encode(char32_t ch, u16 (&buf)[2]) -> usize {
  if (ch <= 0xFFFF) {
    buf[0] = static_cast<char16_t>(ch);
    return 1;
  }

  if (ch <= 0x10FFFF) {
    const auto t = ch - 0x10000;
    buf[0] = static_cast<char16_t>(0xD800 + (t >> 10));
    buf[1] = static_cast<char16_t>(0xDC00 + (t & 0x3FF));
    return 2;
  }

  return 0;
}

auto utf16_decode(const u16 p[], usize n) -> char32_t {
  switch (n) {
    case 1: {
      const auto a = p[0];
      return static_cast<char32_t>(a);
    }
    case 2: {
      const auto a = p[0] & 0x3FF;
      const auto b = p[1] & 0x3FF;
      return static_cast<char32_t>(0x10000 + (a << 10) + b);
    }
  }
  return chr::INVALID;
}

auto wide_codelen(wchar_t h) -> usize {
  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    return 1;
  } else {
    return chr::utf16_codelen(static_cast<u16>(h));
  }
}

auto wide_encode(char32_t ch, wchar_t (&buf)[2]) -> usize {
  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    buf[0] = static_cast<wchar_t>(ch);
    return 1;
  } else {
    return chr::utf16_encode(ch, reinterpret_cast<u16(&)[2]>(buf));
  }
}

auto wide_decode(const wchar_t p[], usize n) -> char32_t {
  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    return static_cast<char32_t>(p[0]);
  } else {
    return chr::utf16_decode(reinterpret_cast<const u16*>(p), n);
  }
}

auto Chars::next() noexcept -> Option<char32_t> {
  if (_ptr == _end) return {};

  const auto n = chr::utf8_codelen(*_ptr);
  if (n == 0 || _ptr + n > _end) {
    _ptr = _end;
    return {};
  }

  const auto ch = chr::utf8_decode(_ptr, n);
  _ptr += n;
  return ch;
}

auto Chars::next_back() noexcept -> Option<char32_t> {
  if (_ptr == _end) return {};

  // rev find the start of the last code point
  auto p = _end - 1;
  while (p > _ptr && chr::utf8_is_continuation(*p)) {
    --p;
  }

  const auto n = chr::utf8_codelen(*p);
  if (p + n != _end) {
    _end = p;
    return {};
  }

  const auto ch = chr::utf8_decode(p, n);
  _end = p;
  return ch;
}

auto WChars::next() noexcept -> Option<char32_t> {
  if (_ptr == _end) return {};

  const auto n = chr::wide_codelen(*_ptr);
  if (n == 0 || _ptr + n > _end) {
    _ptr = _end;
    return {};
  }

  const auto ch = chr::wide_decode(_ptr, n);
  _ptr += n;
  return ch;
}

}  // namespace sfc::chr
