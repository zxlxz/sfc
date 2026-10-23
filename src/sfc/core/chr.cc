#include "sfc/core/chr.h"
#include "sfc/core/str.h"

namespace sfc::chr {

namespace utf8 {
static constexpr u8 H0 = 0x00;  // 0b0xxxxxxx
static constexpr u8 H1 = 0x80;  // 0b10xxxxxx
static constexpr u8 H2 = 0xC0;  // 0b110xxxxx
static constexpr u8 H3 = 0xE0;  // 0b1110xxxx
static constexpr u8 H4 = 0xF0;  // 0b11110xxx
static constexpr u8 H5 = 0xF8;  // 0b111110xx

static auto is_continuation(u8 b) -> bool {
  return (b & H2) == utf8::H1;
}

static auto codelen(u8 h) -> u32 {
  if ((h & H1) == H0) return 1;
  if ((h & H3) == H2) return 2;
  if ((h & H4) == H3) return 3;
  if ((h & H5) == H4) return 4;
  return 0;
}

// ch: [0x000000, 0x10FFFF]
static auto encode(Slice<u8> buf, char32_t ch) -> u32 {
  static constexpr u8 MASK = 0x3F;  // 0b00111111

  if (ch <= 0x7FU && buf._len >= 1) {
    buf._ptr[0] = static_cast<u8>(ch);
    return 1;
  }

  if (ch <= 0x7FFU && buf._len >= 2) {
    buf._ptr[0] = static_cast<u8>(H2 | (MASK & (ch >> 6)));
    buf._ptr[1] = static_cast<u8>(H1 | (MASK & ch));
    return 2;
  }

  if (ch <= 0xFFFFU && buf._len >= 3) {
    buf._ptr[0] = static_cast<u8>(H3 | (MASK & (ch >> 12)));
    buf._ptr[1] = static_cast<u8>(H1 | (MASK & (ch >> 6)));
    buf._ptr[2] = static_cast<u8>(H1 | (MASK & ch));
    return 3;
  }

  if (ch <= 0x10FFFFU && buf._len >= 4) {
    buf._ptr[0] = static_cast<u8>(H4 | (MASK & (ch >> 18)));
    buf._ptr[1] = static_cast<u8>(H1 | (MASK & (ch >> 12)));
    buf._ptr[2] = static_cast<u8>(H1 | (MASK & (ch >> 6)));
    buf._ptr[3] = static_cast<u8>(H1 | (MASK & ch));
    return 4;
  }

  return 0;
}

static auto decode(Slice<const u8> buf) -> char32_t {
  static constexpr char32_t MX = 0x3F;  // 0b00111111
  static constexpr char32_t M2 = 0x1F;  // 0b00011111
  static constexpr char32_t M3 = 0x0F;  // 0b00001111
  static constexpr char32_t M4 = 0x07;  // 0b00000111

  const auto p = buf._ptr;
  switch (buf._len) {
    case 1: {
      const auto a = p[0];
      return char32_t{a};
    }
    case 2: {
      const auto a = p[0] & M2;  // 0b1110000
      const auto b = p[1] & MX;  // 0b00111111
      return char32_t{(a << 6) | b};
    }
    case 3: {
      const auto a = p[0] & M3;  // 0b11110000
      const auto b = p[1] & MX;  // 0b00111111
      const auto c = p[2] & MX;  // 0b00111111
      return char32_t{(a << 12) | (b << 6) | c};
    }
    case 4: {
      const auto a = p[0] & M4;  // 0b11100000
      const auto b = p[1] & MX;  // 0b00111111
      const auto c = p[2] & MX;  // 0b00111111
      const auto d = p[3] & MX;  // 0b00111111
      return char32_t{(a << 18) | (b << 12) | (c << 6) | d};
    }
  }
  return chr::INVALID;
}

}  // namespace utf8

namespace utf16 {
static constexpr u16 H1 = 0xD800;  // 0b11011000'00000000
static constexpr u16 H2 = 0xDC00;  // 0b11011100'00000000

static auto codelen(u16 h) -> u32 {
  if (h < 0xD800) return 1;  // [0x0000, 0xD7FF]  single code unit
  if (h < 0xDBFF) return 2;  // [0xD800, 0xDBFF]  high surrogate
  if (h < 0xE000) return 0;  // [0xDC00, 0xDFFF]  invalid
  return 1;                  // [0xE000, 0xFFFF]  single code unit
}

static auto encode(Slice<u16> buf, char32_t ch) -> u32 {
  static constexpr char32_t NBIT = 10;     // 10bit
  static constexpr char32_t MASK = 0x3FF;  // 10bit
  static constexpr char32_t C1 = 0x10000;  // max(u16) + 1

  if (ch < C1 && buf._len >= 1) {
    buf._ptr[0] = static_cast<u16>(ch);
    return 1;
  }

  if (ch <= 0x10FFFFu && buf._len >= 2) {
    const auto t = ch - C1;
    const auto t0 = t >> NBIT;  // top 10 bits
    const auto t1 = t & MASK;   // low 10
    buf._ptr[0] = static_cast<u16>(H1 + t0);
    buf._ptr[1] = static_cast<u16>(H2 + t1);
    return 2;
  }

  return 0;
}

static auto decode(Slice<const u16> buf) -> char32_t {
  static constexpr char32_t MASK = 0x3FF;  // 10bit

  const auto p = buf._ptr;
  switch (buf._len) {
    case 1: {
      const auto ch = p[0];
      return char32_t{ch};
    }
    case 2: {
      const auto c0 = MASK & p[0];
      const auto c1 = MASK & p[1];
      const auto ch = (c0 << 10) + c1;
      return char32_t{0x10000U + ch};
    }
  }
  return chr::INVALID;
}

}  // namespace utf16

auto Chars::next() noexcept -> Option<char32_t> {
  if (_buf._len == 0) {
    return {};
  }

  const auto n = utf8::codelen(*_buf._ptr);
  if (n > _buf._len) {
    _buf = {};
    return {};
  }

  // invalid code point length
  if (n == 0) {
    _buf = {_buf._ptr + 1, _buf._len - 1};
    return chr::INVALID;
  }

  const auto ch = utf8::decode({_buf._ptr, n});
  _buf = {_buf._ptr + n, _buf._len - n};
  return ch;
}

auto Chars::next_back() noexcept -> Option<char32_t> {
  if (_buf._len == 0) {
    return {};
  }

  // reverse find the start of the last code point
  auto p = _buf._ptr + _buf._len - 1;
  while (p > _buf._ptr && utf8::is_continuation(*p)) {
    --p;
  }

  const auto u8_len = utf8::codelen(*p);

  // not enough bytes for the last code point
  if (_buf._len < u8_len) {
    _buf = {};
    return {};
  }

  // invalid code point at the end of the buffer
  if (p + u8_len != _buf._ptr + _buf._len) {
    _buf._len -= 1;
    return chr::INVALID;
  }

  _buf._len -= u8_len;
  const auto ch = utf8::decode({p, u8_len});
  return ch;
}

auto U16Chars::next() noexcept -> Option<char32_t> {
  if (_buf._len == 0) {
    return {};
  }

  const auto n = utf16::codelen(*_buf._ptr);
  if (_buf._len < n) {
    _buf = {};
    return {};
  }

  const auto ch = utf16::decode({_buf._ptr, n});
  _buf = {_buf._ptr + n, _buf._len - n};
  return ch;
}

auto encode_utf8(Slice<u8> buf, char32_t ch) -> Slice<u8> {
  const auto n = utf8::encode(buf, ch);
  return {buf._ptr, n};
}

auto decode_utf8(Slice<const u8> buf) -> Chars {
  return Chars{{}, buf};
}

auto encode_utf16(Slice<u16> buf, char32_t ch) -> Slice<u16> {
  const auto n = utf16::encode(buf, ch);
  return {buf._ptr, n};
}

auto decode_utf16(Slice<const u16> buf) -> U16Chars {
  return U16Chars{{}, buf};
}

}  // namespace sfc::chr
