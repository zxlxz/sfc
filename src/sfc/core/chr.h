#pragma once

#include "sfc/core/iter.h"
#include "sfc/core/option.h"

namespace sfc::chr {

static constexpr char32_t INVALID = 0xFFFD;

auto utf8_codelen(u8 h) -> usize;
auto utf8_encode(u8 (&buf)[4], char32_t ch) -> usize;
auto utf8_decode(const u8 buf[], usize buf_len) -> char32_t;

auto utf16_codelen(u16 h) -> usize;
auto utf16_encode(u16 (&buf)[2], char32_t ch) -> usize;
auto utf16_decode(const u16 buf[], usize buf_len) -> char32_t;

constexpr auto is_number(char ch) noexcept -> bool {
  return '0' <= ch && ch <= '9';
}

constexpr auto is_alpha(char ch) noexcept -> bool {
  const auto lower = ch | 32;  // to lower
  return ('a' <= lower && lower <= 'z');
}

struct Chars : iter::Iterator<char32_t> {
  const u8* _ptr;
  const u8* _end;

 public:
  Chars(const u8* p, usize n) noexcept : _ptr{p}, _end{p + n} {}
  auto next() noexcept -> Option<char32_t>;
  auto next_back() noexcept -> Option<char32_t>;
};

}  // namespace sfc::chr
