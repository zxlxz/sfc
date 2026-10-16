#pragma once

#include "sfc/core/iter.h"
#include "sfc/core/slice.h"
#include "sfc/core/option.h"

namespace sfc::chr {

constexpr char32_t INVALID = 0xFFFD;

struct Decode {
  char32_t ch;
  u32 len;
};

auto encode_utf8(Slice<u8> buf, char32_t ch) -> str::Str;
auto decode_utf8(Slice<const u8> buf) -> Decode;

auto encode_utf16(Slice<u16> buf, char32_t ch) -> Slice<u16>;
auto decode_utf16(Slice<const u16> buf) -> Decode;

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

struct Chars : iter::Iterator<char32_t> {
  const u8* _ptr;
  const u8* _end;

 public:
  Chars(const u8* p, usize n) noexcept : _ptr{p}, _end{p + n} {}

 public:
  auto len() const -> usize;
  auto next() noexcept -> Option<char32_t>;
  auto next_back() noexcept -> Option<char32_t>;
};

}  // namespace sfc::chr
