#pragma once

#include "sfc/core/option.h"
#include "sfc/core/iter.h"

namespace sfc::chr {

static constexpr auto INVALID = char32_t{0xFFFD};

auto utf8_codelen(u8 h) -> usize;
auto utf8_encode(char32_t ch, u8 (&buf)[4]) -> usize;
auto utf8_decode(const u8 p[], usize n) -> char32_t;

auto utf16_codelen(u16 h) -> usize;
auto utf16_encode(char32_t ch, u16 (&buf)[2]) -> usize;
auto utf16_decode(const u16 p[], usize n) -> char32_t;

auto wide_codelen(wchar_t h) -> usize;
auto wide_encode(char32_t ch, wchar_t (&buf)[2]) -> usize;
auto wide_decode(const wchar_t p[], usize n) -> char32_t;

struct Chars : iter::Iterator<char32_t> {
  const u8* _ptr;
  const u8* _end;

 public:
  Chars(const u8* p, usize n) noexcept : _ptr{p}, _end{p + n} {}

  auto next() noexcept -> Option<char32_t>;
  auto next_back() noexcept -> Option<char32_t>;
};

struct WChars : iter::Iterator<wchar_t> {
  const wchar_t* _ptr;
  const wchar_t* _end;

 public:
  auto next() noexcept -> Option<wchar_t>;
};

}  // namespace sfc::chr
