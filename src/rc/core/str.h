#pragma once

#include "rc/core/slice.h"

namespace rc::str {

using Bytes = slice::Iter<const u8>;

struct CharsBase {};

struct Chars : iter::Iterator<Chars> {
  using Item = u8;

  const u8* _itr;
  const u8* _end;

  constexpr Chars(const u8* itr, const u8* end) noexcept
      : _itr{itr}, _end{end} {}

  auto next() noexcept -> Option<u8> {
    if (_itr != _end) {
      return {*_itr++};
    }
    return {};
  }
};

namespace pattern {

template <class T>
struct Pattern {
  auto find(const Str&) const noexcept -> Option<usize>;
  auto rfind(const Str&) const noexcept -> Option<usize>;
  auto starts_with(const Str&) const noexcept -> bool;
  auto ends_with(const Str&) const noexcept -> bool;
};

Pattern(char8_t)->Pattern<u8>;
Pattern(char)->Pattern<u8>;

Pattern(const Str&)->Pattern<Str>;

template <usize N>
Pattern(const char8_t (&)[N])->Pattern<Str>;

template <usize N>
Pattern(const char (&)[N])->Pattern<Str>;

}  // namespace pattern

template <class T>
struct FromStr {
  static auto from_str(const Str& s) noexcept -> Option<T>;
};

template <class P>
struct Split;

struct Str {
  using Cursor = Slice<const u8>::Cursor;
  using Iter = Slice<const u8>::Iter;

  Slice<const u8> _inner;

  constexpr explicit Str(const u8* p, usize n) : _inner{p, n} {}

  constexpr explicit Str(const Slice<const u8>& s) noexcept : _inner{s} {}

  template <usize N>
  constexpr Str(const char8_t (&s)[N]) noexcept
      : _inner{ptr::cast<u8>(s), N - 1} {}

  template <usize N>
  constexpr Str(const char (&s)[N]) noexcept
      : _inner{ptr::cast<u8>(s), N - 1} {}

  static auto from_cstr(const char* p) noexcept -> Str {
    if (p == nullptr) {
      return Str{nullptr, 0};
    }
    const auto n = ::__builtin_strlen(p);
    const auto s = Slice{ptr::cast<u8>(p), n};
    return Str{s};
  }

  constexpr auto len() const noexcept -> usize { return _inner._len; }

  constexpr auto as_ptr() const noexcept -> const u8* { return _inner._ptr; }

  constexpr auto as_bytes() const noexcept -> const Slice<const u8>& {
    return _inner;
  }

  constexpr auto is_empty() const -> bool { return _inner.is_empty(); }

  constexpr auto chars() const noexcept -> Chars {
    return Chars{_inner._ptr, _inner._ptr + _inner._len};
  }

  auto iter() const noexcept -> Iter { return _inner.iter(); }
  auto begin() const noexcept -> Cursor { return _inner.begin(); }
  auto end() const noexcept -> Cursor { return _inner.end(); }

  auto get_unchecked(usize idx) const -> const u8& {
    return _inner.get_unchecked(idx);
  }

  auto operator[](usize idx) const -> const u8& { return _inner[idx]; }

  auto slice_unchecked(usize start, usize end) const noexcept -> Str {
    return Str{_inner.slice_unchecked(start, end)};
  }

  auto slice(usize start, usize end) const noexcept -> Str {
    return Str{_inner.slice(start, end)};
  }

  auto slice_from(usize start) const noexcept -> Str {
    return Str{_inner.slice_from(start)};
  }

  auto split_at(usize mid) const -> Tuple<Str, Str> {
    auto [a, b] = _inner.split_at(mid);
    return {Str{a}, Str{b}};
  }

  auto eq(const Str& other) const noexcept -> bool {
    if (_inner._len != other._inner._len) return false;
    return cmp::all_eq(_inner._ptr, other._inner._ptr, _inner._len);
  }

  auto partial_cmp(const Str& other) const noexcept -> isize {
    const auto n = cmp::min(_inner._len, other._inner._len);
    const auto r = cmp::partial_cmp(_inner._ptr, other._inner._ptr, n);
    if (r == 0) {
      if (_inner._len == other._inner._len) return 0;
      if (_inner._len < other._inner._len) return -isize(n);
      return isize(n);
    }
    return r;
  }

  template <class P>
  auto split(const P& pat) const -> Split<P> {
    return {*this, pat};
  }

  template <class P>
  auto rsplit(const P& pat) const -> Tuple<Str, Str> {
    const auto p = pattern::Pattern{pat};
    const auto mid = p.rfind(*this).unwrap_or(_inner._len);
    return this->split_at(mid);
  }

  template <class P>
  auto find(const P& pat) const noexcept -> Option<usize> {
    const auto p = pattern::Pattern{pat};
    return p.find(*this);
  }

  template <class P>
  auto rfind(const P& pat) const noexcept -> Option<usize> {
    const auto p = pattern::Pattern{pat};
    return p.rfind(*this);
  }

  template <class P>
  auto contains(const P& pat) const noexcept -> bool {
    const auto res = this->find(pat);
    return res.is_some();
  }

  template <class P>
  auto starts_with(const P& pat) const noexcept -> bool {
    const auto p = pattern::Pattern{pat};
    return p.starts_with(*this);
  }

  template <class P>
  auto ends_with(const P& pat) const noexcept -> bool {
    const auto p = pattern::Pattern{pat};
    return p.ends_with(*this);
  }

  template <class T>
  auto parse() const -> Option<T> {
    return FromStr<T>::from_str(*this);
  }
};

template <class P>
struct Split {
  Str _str;
  P _pat;

  auto next() -> Option<Str> {
    while (!_str.is_empty()) {
      const auto p = pattern::Pattern{_pat};
      const auto x = p.find(_str);
      if (x.is_none()) break;
      if (x._val != 0) {
        return {_str.slice_unchecked(0, x._val)};
      }
      _str = _str.slice_unchecked(1, _str._inner._len);
    }
    return {};
  }
};

#pragma region impl : pattern
namespace pattern {
template <>
struct Pattern<u8> {
  u8 _inner;

  auto find(const Str& s) const noexcept -> Option<usize> {
    return s.as_bytes().find(_inner);
  }

  auto rfind(const Str& s) const noexcept -> Option<usize> {
    return s.as_bytes().rfind(_inner);
  }

  auto starts_with(const Str& s) const noexcept -> bool {
    return !s.is_empty() && s.get_unchecked(0) == _inner;
  }

  auto ends_with(const Str& s) const noexcept -> bool {
    return !s.is_empty() && s.get_unchecked(s._inner._len - 1) == _inner;
  }

  auto split(const Str& s) const noexcept -> Split<u8> {
    return Split<u8>{s, _inner};
  }
};

template <>
struct Pattern<Str> {
  Str _inner;

  auto find(const Str& s) const noexcept -> Option<usize> {
    const auto n = s.len();
    const auto m = _inner.len();
    if (m == 0) return {0};
    if (m > n) return {};

    for (usize i = 0; i < n - m; ++i) {
      if (_inner == s.slice_unchecked(i, i + m)) return {i};
    }
    return {};
  }

  auto rfind(const Str& s) const noexcept -> Option<usize> {
    const auto n = s.len();
    const auto m = _inner.len();

    if (m == 0) return {0};
    for (usize i = n - m; i >= m; --i) {
      if (_inner == s.slice_unchecked(i, i + m)) return {i};
    }
    return {};
  }

  auto starts_with(const Str& s) const noexcept -> bool {
    const auto m = _inner.len();
    const auto n = s.len();

    if (m == 0) return true;
    if (n >= m) return _inner == s.slice_unchecked(0, m);
    return false;
  }

  auto ends_with(const Str& s) const noexcept -> bool {
    const auto m = _inner.len();
    const auto n = s.len();

    if (m == 0) return true;
    if (n >= m) return _inner == s.slice_unchecked(n - m, n);
    return false;
  }
};
}  // namespace pattern
#pragma endregion

#pragma region enum
template <class T, usize N>
auto _parse_enum(const Str& name, const Str (&vals)[N]) -> Option<T> {
  for (usize i = 0; i < N; ++i) {
    if (name.eq(vals[i])) {
      return {T(i)};
    }
  }
  return {};
}
#pragma endregion
}  // namespace rc::str

namespace rc {
using str::Chars;
using str::Str;
}  // namespace rc
