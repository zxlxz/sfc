#pragma once

#include "sfc/core/chr.h"
#include "sfc/core/slice.h"

namespace sfc::str {

struct Str {
  const char* _ptr = nullptr;
  usize       _len = 0;

 public:
  constexpr Str() = default;

  constexpr Str(const char* p, usize n) noexcept : _ptr{p}, _len{n} {}

  template <usize N>
  constexpr Str(const char (&s)[N]) noexcept : _ptr{s}, _len{N - 1} {}

  static constexpr auto from_cstr(const char* p) -> Str {
    return Str{p, p ? __builtin_strlen(p) : 0};
  }

  static constexpr auto from_chars(slice::Slice<const char> s) -> Str {
    return Str{s._ptr, s._len};
  }

  static constexpr auto from_bytes(slice::Slice<const u8> s) -> Str {
    return Str{reinterpret_cast<const char*>(s._ptr), s._len};
  }

  auto as_ptr() const -> const char* {
    return _ptr;
  }

  auto is_empty() const -> bool {
    return _len == 0;
  }

  auto len() const -> usize {
    return _len;
  }

  operator bool() const {
    return _len != 0;
  }

  auto as_chars() const -> slice::Slice<const char> {
    return {_ptr, _len};
  }

  auto as_bytes() const -> slice::Slice<const u8> {
    return {reinterpret_cast<const u8*>(_ptr), _len};
  }

 public:
  auto get_unchecked(usize idx) const -> char {
    return _ptr[idx];
  }

  auto get_unchecked(Range ids) const -> Str {
    return {_ptr + ids._start, ids.len()};
  }

  auto operator[](usize idx) const -> char {
    return idx < _len ? _ptr[idx] : char(0);
  }

  auto operator[](Range ids) const -> Str {
    ids = ids % _len;
    return Str{_ptr + ids._start, ids.len()};
  }

  auto split_at(usize mid) const -> tuple::Tuple<Str, Str> {
    const auto pos = cmp::min(mid, _len);
    const auto a = Str{_ptr, pos};
    const auto b = Str{_ptr + mid, _len - mid};
    return tuple::Tuple{a, b};
  }

  auto iter() const -> slice::Iter<const char> {
    return this->as_chars().iter();
  }

  auto operator==(Str other) const -> bool {
    if (_len != other._len)
      return false;

    const auto ret = __builtin_memcmp(_ptr, other._ptr, _len);
    return ret == 0;
  }

 public:
  auto find(auto&& p) const -> option::Option<usize>;
  auto rfind(auto&& p) const -> option::Option<usize>;

  auto contains(auto&& p) const -> bool;
  auto starts_with(auto&& p) const -> bool;
  auto ends_with(auto&& p) const -> bool;

  auto trim_start_matches(auto&& p) const -> Str;
  auto trim_end_matches(auto&& p) const -> Str;
  auto trim_matches(auto&& p) const -> Str;

  auto split(auto&& p) const;

  auto trim_start() const -> Str {
    return this->trim_start_matches(chr::is_whitespace);
  }

  auto trim_end() const -> Str {
    return this->trim_end_matches(chr::is_whitespace);
  }

  auto trim() const -> Str {
    return this->trim_matches(chr::is_whitespace);
  }

  template <class T>
  auto parse() const -> option::Option<T>;

  void fmt(auto& f) const {
    f.pad(*this);
  }
};


template <class T>
struct FromStr {
  static auto from_str(Str) -> option::Option<T>;
};

template <class T>
auto Str::parse() const -> option::Option<T> {
  if constexpr (requires { T::from_str(*this); }) {
    return T::from_str(*this);
  } else {
    return FromStr<T>::from_str(*this);
  }
}

}  // namespace sfc::str
