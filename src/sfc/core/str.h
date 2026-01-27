#pragma once

#include "sfc/core/num.h"
#include "sfc/core/hash.h"
#include "sfc/core/slice.h"
#include "sfc/core/convert.h"

namespace sfc::str {

struct Str {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  constexpr Str() noexcept = default;

  constexpr Str(const char* s, usize n) noexcept : _ptr{s}, _len{n} {}

  constexpr Str(const char* s) noexcept : _ptr{s}, _len{s ? __builtin_strlen(s) : 0} {}

  constexpr static auto from_utf8(Slice<const u8> s) noexcept -> Str {
    const auto p = reinterpret_cast<const char*>(s._ptr);
    return {p, s._len};
  }

  constexpr auto as_ptr() const noexcept -> const u8* {
    return reinterpret_cast<const u8*>(_ptr);
  }

  constexpr auto is_empty() const noexcept -> bool {
    return _len == 0;
  }

  constexpr auto len() const noexcept -> usize {
    return _len;
  }

  constexpr auto as_str() const noexcept -> Str {
    return *this;
  }

  auto as_bytes() const noexcept -> Slice<const u8> {
    return {static_cast<const u8*>(static_cast<const void*>(_ptr)), _len};
  }

 public:
  constexpr auto operator[](usize idx) const noexcept -> char {
    return _ptr && idx < _len ? _ptr[idx] : '\0';
  }

  constexpr auto operator[](ops::Range ids) const noexcept -> Str {
    const auto start = ids.start < _len ? ids.start : _len;
    const auto end = ids.end < _len ? ids.end : _len;
    return Str{_ptr + start, start < end ? end - start : 0U};
  }

  auto split_at(usize mid) const noexcept -> Tuple<Str, Str> {
    const auto x = mid < _len ? mid : _len;
    return Tuple{Str{_ptr, x}, Str{_ptr + x, _len - x}};
  }

 public:
  using Iter = slice::Iter<const char>;
  auto iter() const noexcept -> slice::Iter<const char> {
    return Iter{{}, _ptr, _ptr + _len};
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
    if (_len != other._len) {
      return false;
    }
    if (_len != 0) {
      return __builtin_memcmp(_ptr, other._ptr, _len) == 0;
    }
    return true;
  }

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

  // trait: str::FromStr
  template <class T>
  auto parse() const -> Option<T> {
    if constexpr (requires { T::from_str(*this); }) {
      return T::from_str(*this);
    } else if constexpr (trait::int_<T>) {
      return num::int_from_str<T>(*this);
    } else if constexpr (trait::flt_<T>) {
      return num::flt_from_str<T>(*this);
    } else {
      static_assert(false, "Str::parse: unsupported type");
    }
  }

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

template <class>
struct Pattern;

template <>
struct Pattern<char> {
  char _needle;

 public:
  constexpr static auto len() noexcept -> usize {
    return 1;
  }

  auto match(const char* s) const noexcept -> bool {
    return *s == _needle;
  }
};

template <class F>
struct Pattern {
  F _pred;

 public:
  constexpr static auto len() noexcept -> usize {
    return 1;
  }

  auto match(const char* s) noexcept -> bool {
    return _pred(*s);
  }
};

template <>
struct Pattern<Str> {
  Str _needle;

 public:
  constexpr auto len() const noexcept -> usize {
    return _needle._len;
  }

  auto match(const char* s) const noexcept -> bool {
    for (auto i = 0U; i < _needle._len; ++i) {
      if (s[i] != _needle._ptr[i]) {
        return false;
      }
    }
    return true;
  }
};

template <class P>
Pattern(P) -> Pattern<P>;

template <convert::Into<char> C>
Pattern(C) -> Pattern<char>;

template <convert::Into<Str> S>
Pattern(S) -> Pattern<Str>;

auto Str::find(auto&& pat) const -> Option<usize> {
  auto p = Pattern{static_cast<decltype(pat)&&>(pat)};
  const auto n = p.len();
  if (_len < n) {
    return {};
  }

  for (auto i = 0U; i <= _len - n; ++i) {
    if (p.match(_ptr + i)) {
      return i;
    }
  }
  return {};
}

auto Str::rfind(auto&& pat) const -> Option<usize> {
  auto p = Pattern{static_cast<decltype(pat)&&>(pat)};
  const auto n = p.len();
  if (_len < n) {
    return {};
  }

  for (auto i = _len; i >= n; --i) {
    if (p.match(_ptr + i - n)) {
      return i - n;
    }
  }
  return {};
}

auto Str::contains(auto&& pat) const -> bool {
  auto p = Pattern{static_cast<decltype(pat)&&>(pat)};
  const auto n = p.len();
  if (_len < n) {
    return false;
  }

  for (auto i = 0U; i <= _len - n; ++i) {
    if (p.match(_ptr + i)) {
      return true;
    }
  }
  return false;
}

auto Str::starts_with(auto&& pat) const -> bool {
  auto p = Pattern{static_cast<decltype(pat)&&>(pat)};
  const auto n = p.len();
  if (_len < n) {
    return false;
  }
  return p.match(_ptr);
}

auto Str::ends_with(auto&& pat) const -> bool {
  auto p = Pattern{static_cast<decltype(pat)&&>(pat)};
  const auto n = p.len();
  if (_len < n) {
    return false;
  }

  return p.match(_ptr + _len - n);
}

auto Str::trim_start_matches(auto&& pat) const -> Str {
  auto p = Pattern{static_cast<decltype(pat)&&>(pat)};
  const auto n = p.len();
  if (n == 0 || n > _len) {
    return *this;
  }

  auto start = 0U;
  while (start <= _len - n && p.match(_ptr + start)) {
    start += n;
  }
  return Str{_ptr + start, _len - start};
}

auto Str::trim_end_matches(auto&& pat) const -> Str {
  auto p = Pattern{static_cast<decltype(pat)&&>(pat)};
  const auto n = p.len();
  if (n == 0 || n > _len) {
    return *this;
  }

  auto end = _len;
  while (end >= n && p.match(_ptr + end - n)) {
    end -= n;
  }
  return Str{_ptr, end};
}

auto Str::trim_matches(auto&& pat) const -> Str {
  auto p = Pattern{static_cast<decltype(pat)&&>(pat)};
  const auto n = p.len();
  if (n == 0 || n > _len) {
    return *this;
  }

  auto start = 0U;
  while (start <= _len - n && p.match(_ptr + start)) {
    start += n;
  }
  auto end = _len;
  while (end >= start + n && p.match(_ptr + end - n)) {
    end -= n;
  }
  return Str{_ptr + start, end - start};
}

template <class T>
static constexpr Str type_name() {
  constexpr auto S1 = sizeof("Str sfc::str::type_name() [T =");
  constexpr auto S2 = sizeof("]");
  return Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
}

template <auto E>
static constexpr Str enum_name() {
  static constexpr auto SN = str::type_name<decltype(E)>();
  static constexpr auto S1 = sizeof("Str sfc::str::enum_name() [E =");
  static constexpr auto S2 = sizeof("]");
  static constexpr auto ss = Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
  for (auto n = ss._len; n != 0; --n) {
    if (ss._ptr[n - 1] == ':') {
      return Str{ss._ptr + n, ss._len - n};
    }
  }
  return ss;
}

}  // namespace sfc::str

namespace sfc::option {

template <>
struct Inner<str::Str> {
  str::Str _val;

 public:
  Inner(none_t) noexcept : _val{} {}

  Inner(some_t, auto&&... args) noexcept : _val{static_cast<decltype(args)&&>(args)...} {}

  explicit operator bool() const noexcept {
    return _val._ptr != nullptr;
  }

  auto operator*() const noexcept -> const str::Str& {
    return _val;
  }

  auto operator*() noexcept -> str::Str& {
    return _val;
  }
};

template <usize N>
Option(const char (&)[N]) -> Option<str::Str>;
}  // namespace sfc::option

namespace sfc {
using str::Str;
}  // namespace sfc
