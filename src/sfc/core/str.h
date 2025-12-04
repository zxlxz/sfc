#pragma once

#include "sfc/core/slice.h"
#include "sfc/io/mod.h"

namespace sfc::str {

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Str() noexcept = default;

  constexpr Str(const char* pat, usize n) noexcept : _ptr{pat}, _len{n} {}

  template <usize N>
  constexpr Str(const char (&s)[N]) noexcept : _ptr{s}, _len{N - 1} {}

  static constexpr auto from_cstr(const char* s) noexcept -> Str {
    if (s == nullptr) {
      return Str{};
    }
    
    auto n = 0UZ;
    while (s[n] != '\0') {
      ++n;
    }
    return Str{s, n};
  }

  static auto from_utf8(Slice<const u8> s) noexcept -> Str {
    const auto pat = static_cast<const char*>(static_cast<const void*>(s._ptr));
    return {pat, s._len};
  }

  static auto from(const auto& f) noexcept -> Str {
    if constexpr (requires { f.as_str(); }) {
      return f.as_str();
    } else if constexpr (requires { Str{f}; }) {
      return Str{f};
    } else if constexpr (requires { Str::from_cstr(f); }) {
      return Str::from_cstr(f);
    } else if constexpr (requires { Str::from_utf8(f); }) {
      return Str::from_utf8(f);
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
    auto i = 0U;
    while (i < _len && is_space(_ptr[i])) {
      ++i;
    }
    return Str{_ptr + i, _len - i};
  }

  auto trim_end() const noexcept -> Str {
    const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
    auto i = _len;
    while (i > 0 && is_space(_ptr[i - 1])) {
      --i;
    }
    return Str{_ptr, i};
  }

  auto trim() const noexcept -> Str {
    return this->trim_start().trim_end();
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
  auto parse() const -> Option<T>;

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
  auto read(Slice<u8> buf) -> io::Result<usize> {
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
auto Str::parse() const -> Option<T> {
  if constexpr (requires { T::from_str(*this); }) {
    return T::from_str(*this);
  } else {
    return FromStr<T>::from_str(*this);
  }
}

namespace pattern {

struct CharSearcher {
  Str _haystack;
  char _needle;
  usize _finger = 0U;
  usize _finger_back = _haystack._len;

 public:
  auto step() const noexcept -> usize {
    return 1;
  }

  auto next() noexcept -> Option<usize> {
    if (_finger >= _finger_back) {
      return {};
    }
    if (_haystack[_finger++] != _needle) {
      return {};
    }
    return _finger - 1;
  }

  auto next_back() noexcept -> Option<usize> {
    if (_finger >= _finger_back) {
      return {};
    }
    if (_haystack[--_finger_back] != _needle) {
      return {};
    }
    return _finger_back;
  }

  auto next_match() noexcept -> Option<usize> {
    while (_finger < _finger_back) {
      if (_haystack[_finger++] == _needle) {
        return _finger - 1;
      }
    }
    return {};
  }

  auto next_match_back() -> Option<usize> {
    while (_finger < _finger_back) {
      if (_haystack[--_finger_back] == _needle) {
        return _finger_back;
      }
    }
    return {};
  }

  auto next_reject() noexcept -> Option<usize> {
    while (_finger < _finger_back) {
      if (_haystack[_finger++] != _needle) {
        return _finger - 1;
      }
    }
    return {};
  }

  auto next_reject_back() noexcept -> Option<usize> {
    while (_finger < _finger_back) {
      if (_haystack[--_finger_back] != _needle) {
        return _finger_back;
      }
    }
    return {};
  }
};

template <class P>
struct PredSearcher {
  Str _haystack;
  P _pred;
  usize _finger = 0U;
  usize _finger_back = _haystack._len;

 public:
  auto step() const noexcept -> usize {
    return 1;
  }

  auto next() noexcept -> Option<usize> {
    if (_finger >= _finger_back) {
      return {};
    }
    if (!_pred(_haystack[_finger++])) {
      return {};
    }
    return _finger - 1;
  }

  auto next_back() noexcept -> Option<usize> {
    if (_finger >= _finger_back) {
      return {};
    }
    if (!_pred(_haystack[--_finger_back])) {
      return {};
    }
    return _finger_back;
  }

  auto next_match() noexcept -> Option<usize> {
    while (_finger < _finger_back) {
      if (_pred(_haystack[_finger++])) {
        return _finger - 1;
      }
    }
    return {};
  }

  auto next_match_back() noexcept -> Option<usize> {
    while (_finger < _finger_back) {
      if (_pred(_haystack[--_finger_back])) {
        return _finger_back;
      }
    }
    return {};
  }

  auto next_reject() noexcept -> Option<usize> {
    while (_finger < _finger_back) {
      if (!_pred(_haystack[_finger++])) {
        return _finger - 1;
      }
    }
    return {};
  }

  auto next_reject_back() noexcept -> Option<usize> {
    while (_finger < _finger_back) {
      if (!_pred(_haystack[--_finger_back])) {
        return _finger_back;
      }
    }
    return {};
  }
};

struct StrSearcher {
  Str _haystack;
  Str _needle;
  usize _finger = 0;
  usize _finger_back = _haystack._len;

 public:
  auto step() const noexcept -> usize {
    return _needle._len;
  }

  auto next() noexcept -> Option<usize> {
    if (_finger + _needle._len > _finger_back) {
      return {};
    }
    const auto tmp = Str{_haystack._ptr + _finger, _needle._len};
    if (_needle != tmp) {
      _finger += 1;
      return {};
    }
    const auto res = _finger;
    _finger += _needle._len;
    return res;
  }

  auto next_back() -> Option<usize> {
    if (_finger + _needle._len > _finger_back) {
      return {};
    }

    const auto tmp = Str{_haystack._ptr + _finger_back - _needle._len, _needle._len};
    if (_needle != tmp) {
      _finger_back -= 1;
      return {};
    }
    _finger_back -= _needle._len;
    return _finger_back;
  }

  auto next_match() noexcept -> Option<usize> {
    while (_finger + _needle._len <= _finger_back) {
      if (_needle == Str{_haystack._ptr + _finger, _needle._len}) {
        _finger += _needle._len;
        return _finger - _needle._len;
      }
      _finger += 1;
    }
    return {};
  }

  auto next_match_back() -> Option<usize> {
    while (_finger + _needle._len <= _finger_back) {
      if (_needle == Str{_haystack._ptr + _finger_back - _needle._len, _needle._len}) {
        _finger_back -= _needle._len;
        return _finger_back;
      }
      _finger_back -= 1;
    }
    return {};
  }
};

template <class P>
auto into_searcher(P&& pattern, Str haystack) {
  if constexpr (requires { Str{pattern}; }) {
    return StrSearcher{haystack, pattern};
  } else if constexpr (requires { static_cast<char>(pattern); }) {
    return CharSearcher{haystack, static_cast<char>(pattern)};
  } else if constexpr (requires { pattern(' '); }) {
    return PredSearcher<P>{haystack, static_cast<P&&>(pattern)};
  } else {
    static_assert(false, "into_searcher: unsupported pattern type");
  }
}

}  // namespace pattern

auto Str::find(auto&& pat) const -> Option<usize> {
  auto s = pattern::into_searcher(static_cast<decltype(pat)&&>(pat), *this);
  return s.next_match();
}

auto Str::rfind(auto&& pat) const -> Option<usize> {
  auto s = pattern::into_searcher(static_cast<decltype(pat)&&>(pat), *this);
  return s.next_match_back();
}

auto Str::contains(auto&& pat) const -> bool {
  auto s = pattern::into_searcher(static_cast<decltype(pat)&&>(pat), *this);
  return s.next_match().is_some();
}

auto Str::starts_with(auto&& pat) const -> bool {
  auto s = pattern::into_searcher(static_cast<decltype(pat)&&>(pat), *this);
  return s.next().is_some();
}

auto Str::ends_with(auto&& pat) const -> bool {
  auto s = pattern::into_searcher(static_cast<decltype(pat)&&>(pat), *this);
  return s.next_back().is_some();
}

auto Str::trim_start_matches(auto&& pat) const -> Str {
  auto s = pattern::into_searcher(static_cast<decltype(pat)&&>(pat), *this);
  const auto idx = s.next_reject().unwrap_or(0);
  return Str{_ptr + idx, _len - idx};
}

auto Str::trim_end_matches(auto&& pat) const -> Str {
  auto s = pattern::into_searcher(static_cast<decltype(pat)&&>(pat), *this);
  const auto idx = s.next_reject_back().unwrap_or(_len - 1);
  return Str{_ptr, idx + 1};
}

auto Str::trim_matches(auto&& pat) const -> Str {
  auto s = pattern::into_searcher(static_cast<decltype(pat)&&>(pat), *this);
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

namespace sfc::option {

template <>
struct Inner<str::Str> {
  str::Str _val;

 public:
  [[gnu::always_inline]] auto is_some() const noexcept -> bool {
    return _val._ptr != nullptr;
  }

  [[gnu::always_inline]] auto is_none() const noexcept -> bool {
    return _val._ptr == nullptr;
  }

  [[gnu::always_inline]] auto operator*() const noexcept -> const str::Str& {
    return _val;
  }

  [[gnu::always_inline]] auto operator*() noexcept -> str::Str& {
    return _val;
  }
};

template <usize N>
Option(const char (&)[N]) -> Option<str::Str>;
}  // namespace sfc::option

namespace sfc {
using str::Str;
}  // namespace sfc
