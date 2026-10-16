#pragma once

#include "sfc/alloc/string.h"

namespace sfc::ffi {

struct WStr;
auto encode_wide(Slice<wchar_t> buf, char32_t c) -> WStr;
auto decode_wide(Slice<const wchar_t> buf) -> chr::Decode;

struct WChars : iter::Iterator<char32_t> {
  const wchar_t* _ptr;
  usize _len;

 public:
  WChars(const wchar_t* p, usize n) noexcept : _ptr{p}, _len{n} {}

 public:
  auto len() const -> usize;
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
  void push_wstr(WStr s);
  auto into_string() const -> String;
};

}  // namespace sfc::ffi
