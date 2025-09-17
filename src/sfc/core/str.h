#pragma once

#include "sfc/core/slice.h"

namespace sfc::str {

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Str() noexcept = default;

  constexpr Str(const char* p, usize n) noexcept : _ptr{p}, _len{n} {}

  template <u32 N>
  constexpr Str(const char (&s)[N]) noexcept : _ptr{s}, _len{N - 1} {}

  static auto from_u8(slice::Slice<const u8> s) noexcept -> Str {
    const auto p = reinterpret_cast<const char*>(s._ptr);
    const auto n = (s._len == 0 || p[s._len - 1]) ? s._len : __builtin_strlen(p);
    return Str{p, n};
  }

  static auto from_cstr(const char* s) noexcept -> Str {
    const auto n = s == nullptr ? 0 : __builtin_strlen(s);
    return Str{s, n};
  }

  static auto from(auto&& f) noexcept -> Str {
    if constexpr (requires { Str{f}; }) {
      return Str{static_cast<decltype(f)&&>(f)};
    } else if constexpr (requires { f.as_str(); }) {
      return f.as_str();
    } else {
      return f;
    }
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
    return _ptr && idx < _len ? _ptr[idx] : '\0';
  }

  auto slice(usize start, usize end = static_cast<usize>(-1)) const noexcept -> Str {
    end = end < _len ? end : _len;
    start = start < end ? start : end;
    const auto len = start < end ? end - start : 0U;
    return Str{_ptr + start, len};
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
    if (_ptr == other._ptr || _len == 0) {
      return true;
    }
    return __builtin_memcmp(_ptr, other._ptr, _len) == 0;
  }

 public:
  auto search(auto&& p) const noexcept;
  auto find(auto&& p) const noexcept -> option::Option<usize>;
  auto rfind(auto&& p) const noexcept -> option::Option<usize>;

  auto contains(auto&& p) const noexcept -> bool;
  auto starts_with(auto&& p) const noexcept -> bool;
  auto ends_with(auto&& p) const noexcept -> bool;

  auto trim_start_matches(auto&& p) const noexcept -> Str;
  auto trim_end_matches(auto&& p) const noexcept -> Str;
  auto trim_matches(auto&& p) const noexcept -> Str;

  auto trim_start() const noexcept -> Str {
    return this->trim_start_matches([](char c) -> bool { return c == ' ' || ('\x09' <= c && c <= '\x0d'); });
  }

  auto trim_end() const noexcept -> Str {
    return this->trim_end_matches([](char c) -> bool { return c == ' ' || ('\x09' <= c && c <= '\x0d'); });
  }

  auto trim() const noexcept -> Str {
    return this->trim_matches([](char c) -> bool { return c == ' ' || ('\x09' <= c && c <= '\x0d'); });
  }

 public:
  auto hash() const -> usize {
    auto res = 0xaf63bd4c8601b7dfULL;
    for (auto i = 0UL; i < _len; ++i) {
      res ^= static_cast<u8>(_ptr[i]);
      res *= 0xaf63bd4c8601b7dfULL;
    }
    return res;
  }

  template <class T>
  auto parse() const noexcept -> option::Option<T>;

  void fmt(auto& f) const {
    const auto& s = f._style;
    s._type == '?' ? f.write_char('"') : (void)0;
    f.pad(*this);
    s._type == '?' ? f.write_char('"') : (void)0;
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
    while (_idx < _end) {
      if (_str[_idx++] == _pat) {
        return _idx - 1;
      }
    }
    return {};
  }

  auto next_match_back() -> option::Option<usize> {
    while (_idx < _end) {
      if (_str[--_end] == _pat) {
        return _end;
      }
    }
    return {};
  }

  auto next_reject() noexcept -> option::Option<usize> {
    while (_idx < _end) {
      if (_str[_idx++] != _pat) {
        return _idx - 1;
      }
    }
    return {};
  }

  auto next_reject_back() noexcept -> option::Option<usize> {
    while (_idx < _end) {
      if (_str[--_end] != _pat) {
        return _end;
      }
    }
    return {};
  }
};

template <class P>
struct PredSearcher {
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
    while (_idx < _end) {
      if (_pat(_str[_idx++])) {
        return _idx - 1;
      }
    }
    return {};
  }

  auto next_match_back() noexcept -> option::Option<usize> {
    while (_idx < _end) {
      if (_pat(_str[--_end])) {
        return _end;
      }
    }
    return {};
  }

  auto next_reject() noexcept -> option::Option<usize> {
    while (_idx < _end) {
      if (!_pat(_str[_idx++])) {
        return _idx - 1;
      }
    }
    return {};
  }

  auto next_reject_back() noexcept -> option::Option<usize> {
    while (_idx < _end) {
      if (!_pat(_str[--_end])) {
        return _end;
      }
    }
    return {};
  }
};

struct StrSearcher {
  Str _str;
  Str _pat;
  usize _idx = 0;
  usize _end = _str._len;

 public:
  auto step() const noexcept -> usize {
    return _pat._len;
  }

  auto next() noexcept -> option::Option<usize> {
    if (_idx + _pat._len > _end) {
      return {};
    }
    if (_pat != Str{_str._ptr + _idx, _pat._len}) {
      _idx += 1;
      return {};
    }
    _idx += _pat._len;
    return _idx - _pat._len;
  }

  auto next_back() -> option::Option<usize> {
    if (_idx + _pat._len > _end) {
      return {};
    }

    if (_pat != Str{_str._ptr + _end - _pat._len, _pat._len}) {
      _end -= 1;
      return {};
    }
    _end -= _pat._len;
    return _end;
  }

  auto next_match() noexcept -> option::Option<usize> {
    while (_idx + _pat._len <= _end) {
      if (_pat == Str{_str._ptr + _idx, _pat._len}) {
        _idx += _pat._len;
        return _idx - _pat._len;
      }
      _idx += 1;
    }
    return {};
  }

  auto next_match_back() -> option::Option<usize> {
    while (_idx + _pat._len <= _end) {
      if (_pat == Str{_str._ptr + _end - _pat._len, _pat._len}) {
        _end -= _pat._len;
        return _end;
      }
      _end -= 1;
    }
    return {};
  }
};

template <class P>
struct Pattern {
  P _val;

 public:
  auto len() const noexcept -> usize {
    return 1U;
  }

  auto into_searcher(Str s) && noexcept -> PredSearcher<P> {
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

auto Str::search(auto&& p) const noexcept {
  return Pattern{static_cast<decltype(p)&&>(p)}.into_searcher(*this);
}

auto Str::find(auto&& p) const noexcept -> option::Option<usize> {
  auto s = Pattern{static_cast<decltype(p)&&>(p)}.into_searcher(*this);
  return s.next_match();
}

auto Str::rfind(auto&& p) const noexcept -> option::Option<usize> {
  auto s = Pattern{static_cast<decltype(p)&&>(p)}.into_searcher(*this);
  return s.next_match_back();
}

auto Str::contains(auto&& p) const noexcept -> bool {
  auto s = Pattern{static_cast<decltype(p)&&>(p)}.into_searcher(*this);
  return s.next_match().is_some();
}

auto Str::starts_with(auto&& p) const noexcept -> bool {
  auto s = Pattern{static_cast<decltype(p)&&>(p)}.into_searcher(*this);
  return s.next().is_some();
}

auto Str::ends_with(auto&& p) const noexcept -> bool {
  auto s = Pattern{static_cast<decltype(p)&&>(p)}.into_searcher(*this);
  return s.next_back().is_some();
}

auto Str::trim_start_matches(auto&& p) const noexcept -> Str {
  if (_len == 0) {
    return {};
  }
  auto ser = Pattern{static_cast<decltype(p)&&>(p)}.into_searcher(*this);
  const auto idx = ser.next_reject().unwrap_or(0);
  return Str{_ptr + idx, _len - idx};
}

auto Str::trim_end_matches(auto&& p) const noexcept -> Str {
  if (_len == 0) {
    return {};
  }
  auto ser = Pattern{static_cast<decltype(p)&&>(p)}.into_searcher(*this);
  const auto idx = ser.next_reject_back().unwrap_or(_len - 1);
  return Str{_ptr, idx + 1};
}

auto Str::trim_matches(auto&& p) const noexcept -> Str {
  if (_len == 0) {
    return {};
  }
  auto ser = Pattern{static_cast<decltype(p)&&>(p)}.into_searcher(*this);
  const auto i1 = ser.next_reject_back().unwrap_or(_len - 1);
  const auto i0 = ser.next_reject().unwrap_or(0);
  return Str{_ptr + i0, i1 + 1 - i0};
}

template <class T>
static auto type_name() -> Str {
  static const auto S1 = sizeof("Str sfc::str::type_name() [T = ") - 1;
  static const auto S2 = sizeof("]");
  return {__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
}

}  // namespace sfc::str

namespace sfc::option {
template <usize N>
Option(const char (&s)[N]) -> Option<str::Str>;
}
