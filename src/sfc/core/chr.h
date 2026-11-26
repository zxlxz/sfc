#pragma once

#include "sfc/core/iter.h"
#include "sfc/core/slice.h"
#include "sfc/core/option.h"

namespace sfc::chr {

constexpr char32_t INVALID = 0xFFFD;

struct Chars : iter::Iterator<char32_t> {
  Slice<const u8> _buf;

 public:
  auto next() noexcept -> Option<char32_t>;
  auto next_back() noexcept -> Option<char32_t>;
};

struct U16Chars : iter::Iterator<char32_t> {
  Slice<const u16> _buf;

 public:
  auto next() noexcept -> Option<char32_t>;
};

struct Utf8CodePoint {
  u8 data[4] = {};
  u8 len = 0;
  auto as_str() const -> str::Str;
};

struct Utf16CodePoint {
  u16 data[2] = {};
  u8 len = 0;
  auto as_slice() const -> Slice<const u16>;
};

auto encode_utf8(char32_t ch) -> Utf8CodePoint;
auto encode_utf16(char32_t ch) -> Utf16CodePoint;

auto decode_utf8(Slice<const u8> buf) -> Chars;
auto decode_utf16(Slice<const u16> buf) -> U16Chars;

constexpr auto is_number(char ch) noexcept -> bool {
  return '0' <= ch && ch <= '9';
}

constexpr auto is_alpha(char ch) noexcept -> bool {
  const auto lower = ch | 32;  // to lower
  return ('a' <= lower && lower <= 'z');
}

constexpr auto is_upcase(char ch) noexcept -> bool {
  return 'A' <= ch && ch <= 'Z';
}

constexpr auto is_lowcase(char ch) noexcept -> bool {
  return 'a' <= ch && ch <= 'z';
}

}  // namespace sfc::chr
