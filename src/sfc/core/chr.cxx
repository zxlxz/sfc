#include "sfc/test/test.h"

namespace sfc::chr::test {

SFC_TEST(utf8_codelen) {
  // ASCII (0xxxxxxx)
  sfc::assert_eq(chr::utf8_codelen(0x00), 1U);
  sfc::assert_eq(chr::utf8_codelen(0x41), 1U);
  sfc::assert_eq(chr::utf8_codelen(0x7F), 1U);

  // 2-byte (110xxxxx)
  sfc::assert_eq(chr::utf8_codelen(0xC2), 2U);
  sfc::assert_eq(chr::utf8_codelen(0xDF), 2U);

  // 3-byte (1110xxxx)
  sfc::assert_eq(chr::utf8_codelen(0xE0), 3U);
  sfc::assert_eq(chr::utf8_codelen(0xEF), 3U);

  // 4-byte (11110xxx)
  sfc::assert_eq(chr::utf8_codelen(0xF0), 4U);
  sfc::assert_eq(chr::utf8_codelen(0xF4), 4U);

  // Invalid: continuation byte (10xxxxxx)
  sfc::assert_eq(chr::utf8_codelen(0x80), 0U);
  sfc::assert_eq(chr::utf8_codelen(0xBF), 0U);

  // Invalid: too high (11111xxx)
  sfc::assert_eq(chr::utf8_codelen(0xF8), 0U);
  sfc::assert_eq(chr::utf8_codelen(0xFF), 0U);
}

SFC_TEST(utf8_encode) {
  u8 buf[4] = {};

  // 1-byte: ASCII
  sfc::assert_eq(chr::utf8_encode(U'A', buf), 1U);
  sfc::assert_eq(buf[0], u8{'A'});

  sfc::assert_eq(chr::utf8_encode(0x7F, buf), 1U);
  sfc::assert_eq(buf[0], 0x7F);

  // 2-byte: U+00E9 (é)
  sfc::assert_eq(chr::utf8_encode(U'é', buf), 2U);
  sfc::assert_eq(buf[0], 0xC3);
  sfc::assert_eq(buf[1], 0xA9);

  // 2-byte: boundary U+0080
  sfc::assert_eq(chr::utf8_encode(0x80, buf), 2U);
  sfc::assert_eq(buf[0], 0xC2);
  sfc::assert_eq(buf[1], 0x80);

  // 2-byte: boundary U+07FF
  sfc::assert_eq(chr::utf8_encode(0x7FF, buf), 2U);
  sfc::assert_eq(buf[0], 0xDF);
  sfc::assert_eq(buf[1], 0xBF);

  // 3-byte: U+4E2D (中)
  sfc::assert_eq(chr::utf8_encode(U'中', buf), 3U);
  sfc::assert_eq(buf[0], 0xE4);
  sfc::assert_eq(buf[1], 0xB8);
  sfc::assert_eq(buf[2], 0xAD);

  // 3-byte: boundary U+0800
  sfc::assert_eq(chr::utf8_encode(0x800, buf), 3U);
  sfc::assert_eq(buf[0], 0xE0);
  sfc::assert_eq(buf[1], 0xA0);
  sfc::assert_eq(buf[2], 0x80);

  // 3-byte: boundary U+FFFF
  sfc::assert_eq(chr::utf8_encode(0xFFFF, buf), 3U);
  sfc::assert_eq(buf[0], 0xEF);
  sfc::assert_eq(buf[1], 0xBF);
  sfc::assert_eq(buf[2], 0xBF);

  // 4-byte: U+1F600 (😀)
  sfc::assert_eq(chr::utf8_encode(0x1F600, buf), 4U);
  sfc::assert_eq(buf[0], 0xF0);
  sfc::assert_eq(buf[1], 0x9F);
  sfc::assert_eq(buf[2], 0x98);
  sfc::assert_eq(buf[3], 0x80);

  // 4-byte: boundary U+10000
  sfc::assert_eq(chr::utf8_encode(0x10000, buf), 4U);
  sfc::assert_eq(buf[0], 0xF0);
  sfc::assert_eq(buf[1], 0x90);
  sfc::assert_eq(buf[2], 0x80);
  sfc::assert_eq(buf[3], 0x80);

  // 4-byte: boundary U+10FFFF
  sfc::assert_eq(chr::utf8_encode(0x10FFFF, buf), 4U);
  sfc::assert_eq(buf[0], 0xF4);
  sfc::assert_eq(buf[1], 0x8F);
  sfc::assert_eq(buf[2], 0xBF);
  sfc::assert_eq(buf[3], 0xBF);

  // Invalid: above U+10FFFF
  sfc::assert_eq(chr::utf8_encode(0x110000, buf), 0U);
}

SFC_TEST(utf8_decode) {
  auto decode = [](Slice<const char8_t> s) {
    const auto p = ptr::cast<const u8>(s._ptr);
    return chr::utf8_decode(p, u32(s._len - 1));
  };

  // 1-byte
  sfc::assert_eq(decode(u8"A"), char32_t{'A'});

  // 2-byte: é (0xC3 0xA9)
  sfc::assert_eq(decode(u8"é"), char32_t{U'é'});

  // 3-byte: 中 (0xE4 0xB8 0xAD)
  sfc::assert_eq(decode(u8"中"), char32_t{U'中'});

  // 4-byte: 😀 (0xF0 0x9F 0x98 0x80)
  sfc::assert_eq(decode(u8"😀"), char32_t{0x1F600});

  // Invalid n=0
  sfc::assert_eq(decode(u8""), chr::INVALID);
}

SFC_TEST(utf16_codelen) {
  // BMP (< 0xD800)
  sfc::assert_eq(chr::utf16_codelen(0x0000), 1U);
  sfc::assert_eq(chr::utf16_codelen(0x0041), 1U);
  sfc::assert_eq(chr::utf16_codelen(0xD7FF), 1U);

  // High surrogate range → 2 (surrogate pair)
  sfc::assert_eq(chr::utf16_codelen(0xD800), 2U);
  sfc::assert_eq(chr::utf16_codelen(0xDBFE), 2U);

  // Low surrogate range → 0 (invalid start)
  sfc::assert_eq(chr::utf16_codelen(0xDC00), 0U);
  sfc::assert_eq(chr::utf16_codelen(0xDFFF), 0U);

  // After surrogates
  sfc::assert_eq(chr::utf16_codelen(0xE000), 1U);
  sfc::assert_eq(chr::utf16_codelen(0xFFFF), 1U);
}

SFC_TEST(utf16_encode) {
  u16 buf[2] = {};

  // BMP
  sfc::assert_eq(chr::utf16_encode( U'A', buf), 1U);
  sfc::assert_eq(buf[0], u16{'A'});

  sfc::assert_eq(chr::utf16_encode( U'中', buf), 1U);
  sfc::assert_eq(buf[0], u16{0x4E2D});

  // Supplementary: U+1F600 (😀)
  sfc::assert_eq(chr::utf16_encode(0x1F600, buf), 2U);
  sfc::assert_eq(buf[0], u16{0xD83D});
  sfc::assert_eq(buf[1], u16{0xDE00});

  // Boundary: U+10000
  sfc::assert_eq(chr::utf16_encode(0x10000, buf), 2U);
  sfc::assert_eq(buf[0], u16{0xD800});
  sfc::assert_eq(buf[1], u16{0xDC00});

  // Boundary: U+10FFFF
  sfc::assert_eq(chr::utf16_encode(0x10FFFF, buf), 2U);
  sfc::assert_eq(buf[0], u16{0xDBFF});
  sfc::assert_eq(buf[1], u16{0xDFFF});

  // Invalid
  sfc::assert_eq(chr::utf16_encode(0x110000, buf), 0U);
}

SFC_TEST(utf16_decode) {
  auto decode = [](Slice<const char16_t> s) {
    const auto p = ptr::cast<const u16>(s._ptr);
    return chr::utf16_decode(p, u32(s._len - 1));
  };

  sfc::assert_eq(decode(u"A"), char32_t{U'A'});
  sfc::assert_eq(decode(u"😀"), char32_t{0x1F600});
  sfc::assert_eq(decode(u""), chr::INVALID);
}

}  // namespace sfc::chr::test
