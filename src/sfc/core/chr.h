#pragma once

#include "sfc/core/iter.h"
#include "sfc/core/slice.h"
#include "sfc/core/option.h"

namespace sfc::chr {

constexpr char32_t INVALID = 0xFFFD;

struct Chars : iter::Iterator<char32_t> {
  Slice<const u8> _buf;

 public:
  auto len() const -> usize;
  auto next() noexcept -> Option<char32_t>;
  auto next_back() noexcept -> Option<char32_t>;
};

struct U16Chars : iter::Iterator<char32_t> {
  Slice<const u16> _buf;

 public:
  auto len() const -> usize;
  auto next() noexcept -> Option<char32_t>;
};

auto encode_utf8(Slice<u8> buf, char32_t ch) -> str::Str;

auto encode_utf16(Slice<u16> buf, char32_t ch) -> Slice<u16>;
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
