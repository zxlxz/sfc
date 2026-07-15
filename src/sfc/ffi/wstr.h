#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

auto wide_codelen(wchar_t h) -> u32;
auto wide_encode(char32_t c, Slice<wchar_t> buf) -> u32;
auto wide_decode(const wchar_t wbuf[], u32 n) -> char32_t;

struct WChars : iter::Iterator<char32_t> {
  const wchar_t* _ptr;
  const wchar_t* _end;

 public:
  WChars(const wchar_t* p, usize n) noexcept : _ptr{p}, _end{p + n} {}
  auto next() noexcept -> Option<char32_t>;
};

struct WStr {
  const wchar_t* _ptr;
  usize _len;

 public:
  auto chars() const -> WChars;
  auto to_string() const -> String;
};

class WString {
  using Buf = List<wchar_t>;
  Buf _buf;

 public:
  static auto from(Str s) -> WString;

  auto buf() -> Buf&;
  auto len() const -> usize;
  auto as_ptr() const -> const wchar_t*;
  auto as_wstr() const -> WStr;

 public:
  void clear();
  void push_str(Str s);
  auto into_string() const -> String;
};

}  // namespace sfc::ffi
