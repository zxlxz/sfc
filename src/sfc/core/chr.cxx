#include "sfc/test/test.h"

namespace sfc::chr::test {

SFC_TEST(encode_utf8) {
  auto encode = [&](char32_t c) { return chr::encode_utf8(c); };

  // 1-byte: ASCII
  sfc::assert_eq(encode(U'A').as_str(), Str{"A"});

  sfc::assert_eq(encode(0x7F).as_str(), Str{"\x7F"});

  // 2-byte: U+00E9 (é)
  sfc::assert_eq(encode(U'é').as_str(), Str{"\xC3\xA9"});

  // 2-byte: boundary U+0080
  sfc::assert_eq(encode(0x80).as_str(), Str{"\xC2\x80"});

  // 2-byte: boundary U+07FF
  sfc::assert_eq(encode(0x07FF).as_str(), Str{"\xDF\xBF"});

  // 3-byte: U+4E2D (中)
  sfc::assert_eq(encode(0x4E2D).as_str(), Str{"\xE4\xB8\xAD"});

  // 3-byte: boundary U+0800
  sfc::assert_eq(encode(0x800).as_str(), Str{"\xE0\xA0\x80"});

  // 3-byte: boundary U+FFFF
  sfc::assert_eq(encode(0xFFFF).as_str(), Str{"\xEF\xBF\xBF"});

  // 4-byte: U+1F600 (😀)
  sfc::assert_eq(encode(0x1F600).as_str(), Str{"\xF0\x9F\x98\x80"});

  // 4-byte: boundary U+10000
  sfc::assert_eq(encode(0x10000).as_str(), Str{"\xF0\x90\x80\x80"});

  // 4-byte: boundary U+10FFFF
  sfc::assert_eq(encode(0x10FFFF).as_str(), Str{"\xF4\x8F\xBF\xBF"});

  // Invalid: above U+10FFFF
  sfc::assert_eq(encode(0x110000).as_str(), Str{});
}

SFC_TEST(decode_utf8) {
  auto decode = [](Str s) {
    auto chars = s.chars();
    return chars.next().unwrap_or(chr::INVALID);
  };

  // 1-byte
  sfc::assert_eq(decode("A"), char32_t{'A'});

  // 2-byte: é (0xC3 0xA9)
  sfc::assert_eq(decode("é"), char32_t{U'é'});

  // 3-byte: 中 (0xE4 0xB8 0xAD)
  sfc::assert_eq(decode("中"), char32_t{U'中'});

  // 4-byte: 😀 (0xF0 0x9F 0x98 0x80)
  sfc::assert_eq(decode("😀"), char32_t{0x1F600});

  // Invalid n=0
  sfc::assert_eq(decode(""), chr::INVALID);
}

SFC_TEST(encode_utf16) {
  auto encode = [&](char32_t ch) { return chr::encode_utf16(ch); };

  auto as_u16_slice = [&]<usize N>(const char16_t (&s)[N]) {
    return Slice{(const u16*)s, N-1};
  };

  // BMP
  sfc::assert_eq(encode(U'A').as_slice(), as_u16_slice(u"\u0041"));
  sfc::assert_eq(encode(U'中').as_slice(), as_u16_slice(u"\u4E2D"));

  // Invalid
  sfc::assert_eq(encode(0x110000).as_slice(), Slice<const u16>{});
}

SFC_TEST(decode_utf16) {
  auto decode = [](Slice<const char16_t> v) {
    const auto s = Slice{ptr::cast<const u16>(v._ptr), v._len - 1};
    return chr::decode_utf16(s).next().unwrap_or(chr::INVALID);
  };

  sfc::assert_eq(decode(u"\u0041"), U'A');
  sfc::assert_eq(decode(u"\U0001F600"), U'\U0001F600');
  sfc::assert_eq(decode(u""), chr::INVALID);
}

}  // namespace sfc::chr::test
