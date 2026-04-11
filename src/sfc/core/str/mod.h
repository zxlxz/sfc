#pragma once

#include "sfc/core/convert.h"
#include "sfc/core/hash.h"
#include "sfc/core/num.h"
#include "sfc/core/slice.h"

namespace sfc::str {

struct Chars;

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  [[gnu::always_inline]] constexpr Str() noexcept = default;

  [[gnu::always_inline]] constexpr Str(const char* s, usize n) noexcept : _ptr{s}, _len{n} {}

  [[gnu::always_inline]] constexpr Str(const char* s) noexcept : _ptr{s}, _len{s ? __builtin_strlen(s) : 0} {}

  [[gnu::always_inline]] static auto from_utf8(Slice<const u8> s) noexcept -> Str {
    return {reinterpret_cast<const char*>(s._ptr), s._len};
  }

  [[gnu::always_inline]] constexpr auto ptr() const noexcept -> const char* {
    return _ptr;
  }

  [[gnu::always_inline]] constexpr auto len() const noexcept -> usize {
    return _len;
  }

  [[gnu::always_inline]] constexpr auto as_ptr() const noexcept -> const char* {
    return _ptr;
  }

  [[gnu::always_inline]] constexpr auto as_str() const noexcept -> Str {
    return *this;
  }

  [[gnu::always_inline]] constexpr auto data() const noexcept -> const char* {
    return _ptr;
  }

  [[gnu::always_inline]] constexpr auto size() const noexcept -> usize {
    return _len;
  }

  [[gnu::always_inline]] constexpr auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  [[gnu::always_inline]] auto as_bytes() const noexcept -> Slice<const u8> {
    return {reinterpret_cast<const u8*>(_ptr), _len};
  }

 public:
  [[gnu::always_inline]] constexpr auto operator[](usize idx) const noexcept -> char {
    if (!_ptr || idx >= _len) return 0;
    return _ptr[idx];
  }

  [[gnu::always_inline]] constexpr auto operator[](Range ids) const noexcept -> Str {
    const auto start = ids.start < _len ? ids.start : _len;
    const auto end = ids.end < _len ? ids.end : _len;
    return Str{_ptr + start, start < end ? end - start : 0U};
  }

  [[gnu::always_inline]] auto split_at(usize mid) const noexcept -> Tuple<Str, Str> {
    const auto x = mid < _len ? mid : _len;
    return Tuple{Str{_ptr, x}, Str{_ptr + x, _len - x}};
  }

 public:
  using Iter = slice::Iter<const char>;
  auto iter() const noexcept -> slice::Iter<const char> {
    return Iter{{}, _ptr, _ptr + _len};
  }

  auto chars() const noexcept -> Chars;

 public:
  auto find(auto&& pat) const -> Option<usize>;
  auto rfind(auto&& pat) const -> Option<usize>;

  auto contains(auto&& pat) const -> bool;
  auto starts_with(auto&& pat) const -> bool;
  auto ends_with(auto&& pat) const -> bool;

  auto trim_start_matches(auto&& pat) const -> Str;
  auto trim_end_matches(auto&& pat) const -> Str;
  auto trim_matches(auto&& pat) const -> Str;

  auto trim_start() const noexcept -> Str {
    const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
    return this->trim_start_matches(is_space);
  }

  auto trim_end() const noexcept -> Str {
    const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
    return this->trim_end_matches(is_space);
  }

  auto trim() const noexcept -> Str {
    const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
    return this->trim_matches(is_space);
  }

 public:
  // trait: ops::Eq
  constexpr auto operator==(Str other) const noexcept -> bool {
    if (_len != other._len) return false;
    if (_len == 0) return true;
    const auto ret = __builtin_memcmp(_ptr, other._ptr, _len);
    return ret == 0;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (auto t = f._spec.type(); t == '?' || t == 's') {
      f.write_char('"');
      f.pad(*this);
      f.write_char('"');
    } else {
      f.pad(*this);
    }
  }

  // trait: str::FromStr
  template <class T>
  auto parse() const -> Option<T>;

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    ser.serialize_str(*this);
  }

  // trait: hash::Hash
  auto hash() const noexcept -> usize {
    auto imp = hash::Hasher{};
    for (auto i = 0UL; i < _len; ++i) {
      imp.write_byte(_ptr[i]);
    }
    return imp.finish();
  }
};

template <class T>
struct FromStr {
  static auto from_str(Str) -> Option<T>;
};

template <class T>
auto Str::parse() const -> Option<T> {
  if constexpr (requires { T::from_str(*this); }) {
    return T::from_str(*this);
  } else {
    return FromStr<T>::from_str(*this);
  }
}

}  // namespace sfc::str

namespace sfc {
using str::Str;
}  // namespace sfc
