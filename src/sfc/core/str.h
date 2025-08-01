#pragma once

#include "sfc/core/slice.h"

namespace sfc::str {

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Str() noexcept = default;

  constexpr Str(const u8* p, usize n) noexcept
      : _ptr{static_cast<const char*>((void*)p)}, _len{n} {}

  constexpr Str(const char* p, usize n) noexcept : _ptr{p}, _len{n} {}

  template <usize N>
  constexpr Str(const char (&s)[N]) noexcept : _ptr{s}, _len{N - 1} {}

  template <usize N>
  constexpr Str(char (&s)[N]) noexcept = delete;

  static constexpr auto from_chars(slice::Slice<const char> s) noexcept -> Str {
    return Str{s._ptr, s._len};
  }

  static constexpr auto from_bytes(slice::Slice<const u8> s) noexcept -> Str {
    return Str{s._ptr, s._len};
  }

  static constexpr auto from(const char* p) noexcept -> Str {
    if (p == nullptr) {
      return {};
    }
    return Str{p, __builtin_strlen(p)};
  }

  auto as_ptr() const noexcept -> const char* {
    return _ptr;
  }

  auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  auto len() const noexcept -> usize {
    return _len;
  }

  explicit operator bool() const noexcept {
    return _len != 0;
  }

  auto as_chars() const noexcept -> slice::Slice<const char> {
    return {_ptr, _len};
  }

  auto as_bytes() const noexcept -> slice::Slice<const u8> {
    return {reinterpret_cast<const u8*>(_ptr), _len};
  }

  auto as_str() const noexcept -> Str {
    return *this;
  }

 public:
  auto operator[](usize idx) const noexcept -> char {
    return idx < _len ? _ptr[idx] : '\0';
  }

  auto operator[](ops::Range ids) const noexcept -> Str {
    const auto end = ids._end < _len ? ids._end : _len;
    const auto pos = ids._start < end ? ids._start : end;
    const auto len = pos <= end ? end - pos : 0U;
    return {_ptr + pos, len};
  }

  auto split_at(usize mid) const noexcept -> tuple::Tuple<Str, Str> {
    const auto x = mid < _len ? mid : _len;
    return tuple::Tuple{Str{_ptr, x}, Str{_ptr + x, _len - x}};
  }

  auto iter() const noexcept -> slice::Iter<const char> {
    return this->as_chars().iter();
  }

  auto operator==(Str other) const noexcept -> bool {
    if (_len != other._len) {
      return false;
    }

    const auto ret = __builtin_memcmp(_ptr, other._ptr, _len);
    return ret == 0;
  }

 public:
  auto find(auto&& p) const noexcept -> option::Option<usize>;
  auto rfind(auto&& p) const noexcept -> option::Option<usize>;

  auto contains(auto&& p) const noexcept -> bool;
  auto starts_with(auto&& p) const noexcept -> bool;
  auto ends_with(auto&& p) const noexcept -> bool;

  auto trim_start_matches(auto&& p) const noexcept -> Str;
  auto trim_end_matches(auto&& p) const noexcept -> Str;
  auto trim_matches(auto&& p) const noexcept -> Str;

  auto trim_start() const noexcept -> Str {
    return this->trim_start_matches([](auto c) {
      return c == ' ' || ('\x09' <= c && c <= '\x0d');
    });
  }

  auto trim_end() const noexcept -> Str {
    return this->trim_end_matches([](auto c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); });
  }

  auto trim() const noexcept -> Str {
    return this->trim_matches([](auto c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); });
  }

 public:
  auto hash() const -> usize {
    auto res = 0xaf63bd4c8601b7dfULL;
    for (auto i = 0U; i < _len; ++i) {
      res ^= static_cast<u8>(_ptr[i]);
      res *= 0xaf63bd4c8601b7dfULL;
    }
    return res;
  }

  template <class T>
  auto parse() const noexcept -> option::Option<T>;

  void fmt(auto& f) const {
    f.pad(*this);
  }

  auto serialize(auto& s) const {
    return s.ser_str(*this);
  }
};

template <class T>
struct FromStr {
  static auto from_str(Str) -> option::Option<T>;
};

template <class T>
auto Str::parse() const noexcept -> option::Option<T> {
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
  auto step() const noexcept -> usize {
    return 1;
  }

  auto next() noexcept -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (_str[_idx++] != _pat) {
      return {};
    }

    return _idx - 1;
  }

  auto next_back() noexcept -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (_str[--_end] != _pat) {
      return {};
    }

    return _end;
  }

  auto next_match() noexcept -> option::Option<usize> {
    for (; _idx < _end; ++_idx) {
      if (_str[_idx] == _pat) {
        return _idx;
      }
    }
    return {};
  }

  auto next_match_back() -> option::Option<usize> {
    for (; _idx < _end; --_end) {
      if (_str[_end - 1] == _pat) {
        return _end - 1;
      }
    }

    return {};
  }

  auto next_reject() noexcept -> option::Option<usize> {
    for (; _idx < _end; ++_idx) {
      if (_str[_idx] != _pat) {
        return _idx;
      }
    }
    return {};
  }

  auto next_reject_back() noexcept -> option::Option<usize> {
    for (; _idx < _end; --_end) {
      if (_str[_end - 1] != _pat) {
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
  auto step() const noexcept -> usize {
    return 1;
  }

  auto next() noexcept -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (!_pat(_str[_idx++])) {
      return {};
    }

    return _idx - 1;
  }

  auto next_back() noexcept -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    if (!_pat(_str[--_end])) {
      return {};
    }

    return _end;
  }

  auto next_match() noexcept -> option::Option<usize> {
    for (; _idx < _end; ++_idx) {
      if (_pat(_str[_idx])) {
        return _idx;
      }
    }
    return {};
  }

  auto next_match_back() noexcept -> option::Option<usize> {
    for (; _idx < _end; --_end) {
      if (_pat(_str[_end - 1])) {
        return _end - 1;
      }
    }

    return {};
  }

  auto next_reject() noexcept -> option::Option<usize> {
    for (; _idx < _end; ++_idx) {
      if (!_pat(_str[_idx])) {
        return _idx;
      }
    }
    return {};
  }

  auto next_reject_back() noexcept -> option::Option<usize> {
    for (; _idx < _end; --_end) {
      if (!_pat(_str[_end - 1])) {
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
  auto step() const noexcept -> usize {
    return _pat._len;
  }

  auto next() noexcept -> option::Option<usize> {
    if (_idx >= _end) {
      return {};
    }

    const auto tmp = Str{_str._ptr + _idx, _pat._len};
    if (!(tmp == _pat)) {
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

    const auto tmp = Str{_str._ptr + _end - 1, _pat._len};
    if (!(tmp == _pat)) {
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
  auto len() const noexcept -> usize {
    return 1U;
  }

  auto into_searcher(Str s) && noexcept -> PredSercher<P> {
    return {s, static_cast<P&&>(_val)};
  }
};

template <>
struct Pattern<char> {
  char _val;

 public:
  auto len() const noexcept -> usize {
    return 1U;
  }

  auto into_searcher(Str s) const noexcept -> CharSearcher {
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

template <class P>
auto Str::find(P&& p) const noexcept -> option::Option<usize> {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next_match();
}

template <class P>
auto Str::rfind(P&& p) const noexcept -> option::Option<usize> {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next_match_back();
}

template <class P>
auto Str::contains(P&& p) const noexcept -> bool {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next_match();
}

template <class P>
auto Str::starts_with(P&& p) const noexcept -> bool {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next();
}

template <class P>
auto Str::ends_with(P&& p) const noexcept -> bool {
  auto s = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  return s.next_back();
}

template <class P>
auto Str::trim_start_matches(P&& p) const noexcept -> Str {
  auto ser = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  const auto idx = ser.next_reject().unwrap_or(0);
  return Str{_ptr + idx, _len - idx};
}

template <class P>
auto Str::trim_end_matches(P&& p) const noexcept -> Str {
  auto ser = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  const auto idx = ser.next_reject_back().unwrap_or(_len - 1);
  return Str{_ptr, idx + 1};
}

template <class P>
auto Str::trim_matches(P&& p) const noexcept -> Str {
  auto ser = Pattern{static_cast<P&&>(p)}.into_searcher(*this);
  const auto i1 = ser.next_reject_back().unwrap_or(_len - 1);
  const auto i0 = ser.next_reject().unwrap_or(0);
  return Str{_ptr + i0, i1 + 1 - i0};
}

}  // namespace sfc::str
