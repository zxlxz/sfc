#include "sfc/core/str.h"

namespace sfc::str {

static constexpr auto kMaxUtf8CodeLen = 4U;
static constexpr auto kInvalidChar = char32_t{0xFFFD};

static auto utf8_codelen(u8 h) -> u32 {
  if ((h & 0x80) == 0) {
    return 1;
  } else if ((h & 0xE0) == 0xC0) {
    return 2;
  } else if ((h & 0xF0) == 0xE0) {
    return 3;
  } else if ((h & 0xF8) == 0xF0) {
    return 4;
  }
  return 0;
}

static auto utf8_codepoint_prev(const char* begin, const char* end) -> const char* {
  if (begin + kMaxUtf8CodeLen > end) {
    begin = end - kMaxUtf8CodeLen;
  }

  auto p = end - 1;
  while (p > begin && (p[0] & 0xC0) == 0x80) {
    p--;
  }

  return p;
}

static auto utf8_decode(const char* p, u32 n) -> char32_t {
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
  return kInvalidChar;
}

auto Chars::next() noexcept -> Option<char32_t> {
  if (_ptr >= _end) {
    return {};
  }

  const auto cnt = utf8_codelen(*_ptr);
  // invalid utf8 code-point
  if (cnt == 0) {
    _ptr += 1;
    return kInvalidChar;
  }

  // not enough bytes
  if (_ptr + cnt > _end) {
    _ptr = _end;
    return kInvalidChar;
  }

  const auto ch = utf8_decode(_ptr, cnt);
  _ptr += cnt;

  return ch;
}

auto Chars::next_back() noexcept -> Option<char32_t> {
  if (_ptr >= _end) {
    return {};
  }

  const auto p = utf8_codepoint_prev(_ptr, _end);

  // invalid utf8, cannot find code-point
  if (p == nullptr) {
    _end -= 1;
    return kInvalidChar;
  }

  const auto cnt = utf8_codelen(*p);
  if (cnt == 0) {
    _end -= 1;
    return kInvalidChar;
  }

  // not enough bytes
  if (_ptr + cnt > _end) {
    _ptr = _end;
    return kInvalidChar;
  }

  const auto ch = utf8_decode(_ptr, cnt);
  _ptr += cnt;
  return ch;
}

}  // namespace sfc::str
