#pragma once

#include "sfc/core/slice.h"

namespace sfc::str {

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Str() noexcept = default;

  constexpr Str(const char* p, usize n) noexcept : _ptr{p}, _len{n} {}

  constexpr Str(const u8* p, usize n) noexcept : _ptr{static_cast<const char*>(static_cast<const void*>(p))}, _len{n} {}

  template <usize N>
  constexpr Str(const char (&s)[N]) noexcept : _ptr{s}, _len{N - 1} {}

  static auto from_utf8(Slice<const u8> s) noexcept -> Str {
    const auto p = static_cast<const char*>(static_cast<const void*>(s._ptr));
    return {p, s._len};
  }

  static auto from_cstr(const char* s) noexcept -> Str {
    const auto n = s == nullptr ? 0 : __builtin_strlen(s);
    return Str{s, n};
  }

  static auto from(const auto& f) noexcept -> Str {
    if constexpr (requires { f.as_str(); }) {
      return f.as_str();
    } else if constexpr (requires { Str{f}; }) {
      return Str{f};
    } else if constexpr (requires { static_cast<const char*>(f); }) {
      return Str::from_cstr(f);
    } else {
      static_assert(false, "Str::from: unsupported type");
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

  auto as_chars() const noexcept -> Slice<const char> {
    return {_ptr, _len};
  }

  auto as_bytes() const noexcept -> Slice<const u8> {
    return {reinterpret_cast<const u8*>(_ptr), _len};
  }

  auto as_str() const noexcept -> Str {
    return *this;
  }

 public:
  auto operator[](usize idx) const noexcept -> char {
    return _ptr && idx < _len ? _ptr[idx] : '\0';
  }

  auto operator[](ops::Range ids) const noexcept -> Str {
    const auto start = ids.start < _len ? ids.start : _len;
    const auto end = ids.end < _len ? ids.end : _len;
    return Str{_ptr + start, start < end ? end - start : 0U};
  }

  auto split_at(usize mid) const noexcept -> Tuple<Str, Str> {
    const auto x = mid < _len ? mid : _len;
    return Tuple{Str{_ptr, x}, Str{_ptr + x, _len - x}};
  }

  auto iter() const noexcept -> slice::Iter<const char> {
    return this->as_chars().iter();
  }

  auto operator==(Str other) const noexcept -> bool {
    if (_len != other._len) {
      return false;
    }
    if (_len == 0 || _ptr == other._ptr) {
      return true;
    }
    if (_ptr && other._ptr) {
      return __builtin_memcmp(_ptr, other._ptr, _len) == 0;
    }
    return false;
  }

 public:
  auto search(auto&& p) const noexcept;
  auto find(auto&& p) const noexcept -> Option<usize>;
  auto rfind(auto&& p) const noexcept -> Option<usize>;

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
  auto parse() const noexcept -> Option<T>;

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (f._style._type == '?' || f._style._type == 's') {
      f.write_char('"');
      f.pad(*this);
      f.write_char('"');
    } else {
      f.pad(*this);
    }
  }

  // trait:: io::Read
  auto read(Slice<u8> buf) -> usize {
    const auto amt = _len < buf._len ? _len : buf._len;
    __builtin_memcpy(buf._ptr, _ptr, amt);
    _ptr += amt;
    _len -= amt;
    return amt;
  }

  // trait: serde::Serialize
  void serialize(auto& ser) const {
    ser.serialize_str(*this);
  }
};

template <class T>
struct FromStr {
  static auto from_str(Str) -> Option<T>;
};

template <class T>
auto Str::parse() const noexcept -> Option<T> {
  if constexpr (requires { T::from_str(*this); }) {
    return T::from_str(*this);
  } else {
    return FromStr<T>::from_str(*this);
  }
}

namespace pattern {

struct CharSearcher {
  Str _str;
  char _pat;
  usize _idx = 0U;
  usize _end = _str._len;

 public:
  auto step() const noexcept -> usize {
    return 1;
  }

  auto next() noexcept -> Option<usize> {
    if (_idx >= _end) {
      return {};
    }
    if (_str[_idx++] != _pat) {
      return {};
    }
    return _idx - 1;
  }

  auto next_back() noexcept -> Option<usize> {
    if (_idx >= _end) {
      return {};
    }
    if (_str[--_end] != _pat) {
      return {};
    }
    return _end;
  }

  auto next_match() noexcept -> Option<usize> {
    while (_idx < _end) {
      if (_str[_idx++] == _pat) {
        return _idx - 1;
      }
    }
    return {};
  }

  auto next_match_back() -> Option<usize> {
    while (_idx < _end) {
      if (_str[--_end] == _pat) {
        return _end;
      }
    }
    return {};
  }

  auto next_reject() noexcept -> Option<usize> {
    while (_idx < _end) {
      if (_str[_idx++] != _pat) {
        return _idx - 1;
      }
    }
    return {};
  }

  auto next_reject_back() noexcept -> Option<usize> {
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

  auto next() noexcept -> Option<usize> {
    if (_idx >= _end) {
      return {};
    }
    if (!_pat(_str[_idx++])) {
      return {};
    }
    return _idx - 1;
  }

  auto next_back() noexcept -> Option<usize> {
    if (_idx >= _end) {
      return {};
    }
    if (!_pat(_str[--_end])) {
      return {};
    }
    return _end;
  }

  auto next_match() noexcept -> Option<usize> {
    while (_idx < _end) {
      if (_pat(_str[_idx++])) {
        return _idx - 1;
      }
    }
    return {};
  }

  auto next_match_back() noexcept -> Option<usize> {
    while (_idx < _end) {
      if (_pat(_str[--_end])) {
        return _end;
      }
    }
    return {};
  }

  auto next_reject() noexcept -> Option<usize> {
    while (_idx < _end) {
      if (!_pat(_str[_idx++])) {
        return _idx - 1;
      }
    }
    return {};
  }

  auto next_reject_back() noexcept -> Option<usize> {
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

  auto next() noexcept -> Option<usize> {
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

  auto next_back() -> Option<usize> {
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

  auto next_match() noexcept -> Option<usize> {
    while (_idx + _pat._len <= _end) {
      if (_pat == Str{_str._ptr + _idx, _pat._len}) {
        _idx += _pat._len;
        return _idx - _pat._len;
      }
      _idx += 1;
    }
    return {};
  }

  auto next_match_back() -> Option<usize> {
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
auto into_searcher(P&& p, Str s) {
  if constexpr (requires { Str(p); }) {
    return StrSearcher{s, Str{p}};
  } else if constexpr (requires { static_cast<char>(p); }) {
    return CharSearcher{s, static_cast<char>(p)};
  } else if constexpr (requires { p(' '); }) {
    return PredSearcher<P>{s, static_cast<P&&>(p)};
  } else {
    static_assert(false, "into_searcher: unsupported pattern type");
  }
}

}  // namespace pattern

auto Str::search(auto&& p) const noexcept {
  return pattern::into_searcher(p, *this);
}

auto Str::find(auto&& p) const noexcept -> Option<usize> {
  auto s = pattern::into_searcher(p, *this);
  return s.next_match();
}

auto Str::rfind(auto&& p) const noexcept -> Option<usize> {
  auto s = pattern::into_searcher(p, *this);
  return s.next_match_back();
}

auto Str::contains(auto&& p) const noexcept -> bool {
  auto s = pattern::into_searcher(p, *this);
  return s.next_match().is_some();
}

auto Str::starts_with(auto&& p) const noexcept -> bool {
  auto s = pattern::into_searcher(p, *this);
  return s.next().is_some();
}

auto Str::ends_with(auto&& p) const noexcept -> bool {
  auto s = pattern::into_searcher(p, *this);
  return s.next_back().is_some();
}

auto Str::trim_start_matches(auto&& p) const noexcept -> Str {
  if (_len == 0) {
    return {};
  }
  auto s = pattern::into_searcher(p, *this);
  const auto idx = s.next_reject().unwrap_or(0);
  return Str{_ptr + idx, _len - idx};
}

auto Str::trim_end_matches(auto&& p) const noexcept -> Str {
  if (_len == 0) {
    return {};
  }
  auto s = pattern::into_searcher(p, *this);
  const auto idx = s.next_reject_back().unwrap_or(_len - 1);
  return Str{_ptr, idx + 1};
}

auto Str::trim_matches(auto&& p) const noexcept -> Str {
  if (_len == 0) {
    return {};
  }
  auto s = pattern::into_searcher(p, *this);
  const auto i1 = s.next_reject_back().unwrap_or(_len - 1);
  const auto i0 = s.next_reject().unwrap_or(0);
  return Str{_ptr + i0, i1 + 1 - i0};
}

template <class T>
static auto type_name() -> Str {
  static const auto S1 = sizeof("Str sfc::str::type_name() [T = ") - 1;
  static const auto S2 = sizeof("]");
  static const auto ss = Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
  return ss;
}

template <auto E>
static auto enum_name() -> Str {
  static_assert(trait::enum_<decltype(E)>);
  static const auto S1 = sizeof("Str sfc::str::enum_name() [E = ") - 1;
  static const auto S2 = sizeof("]");
  static const auto ss = {__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
  return ss;
}

}  // namespace sfc::str

namespace sfc {
using str::Str;
}  // namespace sfc
