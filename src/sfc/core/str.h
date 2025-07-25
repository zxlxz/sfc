#pragma once

#include "sfc/core/slice.h"

namespace sfc::str {

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Str() = default;

  constexpr Str(const char* p, usize n) noexcept : _ptr{p}, _len{n} {}

  constexpr Str(const char* p) noexcept : _ptr{p}, _len{p ? __builtin_strlen(p) : 0} {}

  static constexpr auto from_chars(slice::Slice<const char> s) -> Str {
    return Str{s._ptr, s._len};
  }

  static constexpr auto from_bytes(slice::Slice<const u8> s) -> Str {
    const auto p = reinterpret_cast<const char*>(s._ptr);
    return Str{p, s._len};
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

  explicit operator bool() const {
    return _len != 0;
  }

  auto as_chars() const -> slice::Slice<const char> {
    return {_ptr, _len};
  }

  auto as_bytes() const -> slice::Slice<const u8> {
    return {reinterpret_cast<const u8*>(_ptr), _len};
  }

  auto as_str() const -> Str {
    return *this;
  }

 public:
  auto get_unchecked(usize idx) const -> char {
    return _ptr[idx];
  }

  auto operator[](usize idx) const -> char {
    return idx < _len ? _ptr[idx] : '\0';
  }

  auto operator[](ops::Range ids) const -> Str {
    const auto end = cmp::min(ids._end, _len);
    const auto pos = cmp::min(ids._start, end);
    return {_ptr + pos, end - pos};
  }

  auto split_at(usize mid) const -> tuple::Tuple<Str, Str> {
    const auto x = cmp::min(mid, _len);
    return tuple::Tuple{Str{_ptr, x}, Str{_ptr + x, _len - x}};
  }

  auto iter() const -> slice::Iter<const char> {
    return this->as_chars().iter();
  }

  auto operator==(Str other) const -> bool {
    if (_len != other._len) {
      return false;
    }

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
    return this->trim_start_matches([](auto c) {
      return c == ' ' || ('\x09' <= c && c <= '\x0d');
    });
  }

  auto trim_end() const -> Str {
    return this->trim_end_matches([](auto c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); });
  }

  auto trim() const -> Str {
    return this->trim_matches([](auto c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); });
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

struct CharSearcher {
  Str _str;
  char _pat;
  usize _idx = 0U;
  usize _end = _str._len;

 public:
  auto step() const -> usize {
    return 1;
  }

  auto next() -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (_str._ptr[_idx++] != _pat) {
      return {};
    }

    return _idx - 1;
  }

  auto next_back() -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (_str._ptr[--_end] != _pat) {
      return {};
    }

    return _end;
  }

  auto next_match() -> option::Option<usize> {
    for (; _idx < _end; ++_idx) {
      if (_str._ptr[_idx] == _pat) {
        return _idx;
      }
    }
    return {};
  }

  auto next_match_back() -> option::Option<usize> {
    for (; _idx < _end; --_end) {
      if (_str._ptr[_end - 1] == _pat) {
        return _end - 1;
      }
    }

    return {};
  }

  auto next_reject() -> option::Option<usize> {
    for (; _idx < _end; ++_idx) {
      if (_str._ptr[_idx] != _pat) {
        return _idx;
      }
    }
    return {};
  }

  auto next_reject_back() -> option::Option<usize> {
    for (; _idx < _end; --_end) {
      if (_str._ptr[_end - 1] != _pat) {
        return _end - 1;
      }
    }
    return {};
  }
};

template <class P>
struct PredSercher {
  Str _str;
  P _pat;
  usize _idx = 0U;
  usize _end = _str._len;

 public:
  auto step() const -> usize {
    return 1;
  }

  auto next() -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (!_pat(_str._ptr[_idx++])) {
      return {};
    }

    return _idx - 1;
  }

  auto next_back() -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (!_pat(_str._ptr[--_end])) {
      return {};
    }

    return _end;
  }

  auto next_match() -> option::Option<usize> {
    for (; _idx < _end; ++_idx) {
      if (_pat(_str._ptr[_idx])) {
        return _idx;
      }
    }
    return {};
  }

  auto next_match_back() -> option::Option<usize> {
    for (; _idx < _end; --_end) {
      if (_pat(_str._ptr[_end - 1])) {
        return _end - 1;
      }
    }

    return {};
  }

  auto next_reject() -> option::Option<usize> {
    for (; _idx < _end; ++_idx) {
      if (!_pat(_str._ptr[_idx])) {
        return _idx;
      }
    }
    return {};
  }

  auto next_reject_back() -> option::Option<usize> {
    for (; _idx < _end; --_end) {
      if (!_pat(_str._ptr[_end - 1])) {
        return _end - 1;
      }
    }
    return {};
  }
};

struct StrSearcher {
  Str _str;
  Str _pat;
  usize _idx = 0;
  usize _end = _str._len + 1 - _pat._len;

 public:
  auto step() const -> usize {
    return _pat._len;
  }

  auto next() -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (_idx + _pat._len > _end) {
      _idx = _end;
      return {};
    }

    if (__builtin_memcmp(_str._ptr + _idx, _pat._ptr, _pat._len) != 0) {
      _idx += 1;
      return {};
    }

    _idx += _pat._len;
    return _idx - _pat._len;
  }

  auto next_back() -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (_idx + _pat._len > _end) {
      _end = _idx;
      return {};
    }

    if (__builtin_memcmp(_str._ptr + _end - 1, _pat._ptr, _pat._len) != 0) {
      _end -= 1;
      return {};
    }

    _end -= _pat._len;
    return _end + _pat._len - 1;
  }
};

template <class P>
struct Pattern {
  P _val;

 public:
  auto len() const -> usize {
    return 1;
  }

  auto into_searcher(Str s) && -> PredSercher<P> {
    return {s, static_cast<P&&>(_val)};
  }
};

template <>
struct Pattern<char> {
  char _val;

 public:
  auto len() const -> usize {
    return 1;
  }

  auto into_searcher(Str s) const -> CharSearcher {
    return {s, _val};
  }
};

template <>
struct Pattern<Str> {
  Str _val;

 public:
  auto len() const -> usize {
    return _val.len();
  }

  auto into_searcher(Str s) const -> StrSearcher {
    return {s, _val};
  }
};

template <class P>
Pattern(P) -> Pattern<P>;

Pattern(char) -> Pattern<char>;

Pattern(Str) -> Pattern<Str>;

template <usize N>
Pattern(const char (&)[N]) -> Pattern<Str>;

template <usize N>
Pattern(char (&)[N]) -> Pattern<Str>;

template <class S>
struct Split : iter::Iterator<Split<S>, Str> {
  S _searcher;

 public:
  auto next() -> option::Option<Str> {
    const auto cur_pos = _searcher._idx;
    _searcher.next();
  }
};

template <class P>
auto Str::find(P&& p) const -> option::Option<usize> {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next_match();
}

template <class P>
auto Str::rfind(P&& p) const -> option::Option<usize> {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next_match_back();
}

template <class P>
auto Str::contains(P&& p) const -> bool {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next_match();
}

template <class P>
auto Str::starts_with(P&& p) const -> bool {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next();
}

template <class P>
auto Str::ends_with(P&& p) const -> bool {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next_back();
}

template <class P>
auto Str::trim_start_matches(P&& p) const -> Str {
  auto ser = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  const auto cnt = ser.step();
  const auto idx = ser.next_reject();
  return idx ? (*this)[{*idx, cnt}] : Str{};
}

template <class P>
auto Str::trim_end_matches(P&& p) const -> Str {
  auto ser = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  const auto idx = ser.next_reject_back();
  return idx ? (*this)[{0, *idx + 1}] : Str{};
}

template <class P>
auto Str::trim_matches(P&& p) const -> Str {
  auto ser = Pattern{static_cast<P&&>(p)}.into_searcher(*this);

  auto res = *this;
  if (auto idx = ser.next_reject_back()) {
    res = res[{0, *idx}];
  }
  if (auto idx = ser.next_reject()) {
    res = res[{*idx, res.len() - *idx}];
  }
  return res;
}

template <class P>
auto Str::split(P&& p) const {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return Split{*this, s};
}

}  // namespace sfc::str
