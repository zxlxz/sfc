#pragma once

#include "rc/alloc/vec.h"

namespace rc::string {

using str::Str;
using vec::Vec;

template <class T>
struct IntoString;

struct String {
  Vec<u8> _vec;

  String() noexcept = default;
  ~String() noexcept = default;
  String(String&& other) noexcept = default;

  explicit String(Vec<u8>&& vec) noexcept : _vec{rc::move(vec)} {}

  template <class S>
  [[nodiscard]] static auto from(const S& s) -> String {
    return IntoString<S>{s}.into_string();
  }

  [[nodiscard]] pub static auto with_capacity(usize cap) -> String {
    return String{Vec<u8>::with_capacity(cap)};
  }

  [[nodiscard]] auto clone() const -> String {
    String res;
    res._vec.append_elements(_vec.as_slice());
    return res;
  }

  operator Str() const noexcept { return Str{_vec._buf._ptr, _vec._len}; }

  auto as_str() const noexcept -> Str { return Str{_vec._buf._ptr, _vec._len}; }

  auto as_bytes() const noexcept -> Slice<const u8> {
    return {_vec._buf._ptr, _vec._len};
  }

  auto len() const noexcept -> usize { return _vec._len; }
  auto capacity() const noexcept -> usize { return _vec._buf._cap; }
  auto is_empty() const noexcept -> bool { return _vec._len == 0; }

  auto operator[](usize idx) const -> u8 { return _vec[idx]; }
  auto operator[](usize idx) -> u8& { return _vec[idx]; }

  auto slice(usize start, usize end) const -> Str {
    return this->as_str().slice(start, end);
  }

  auto eq(Str s) const noexcept -> bool { return as_str().eq(s); }

  void reserve(usize additional) { _vec.reserve(additional); }

  void truncate(usize new_len) { _vec.truncate(new_len); }

  void shrink_to_fit() { _vec.shrink_to_fit(); }

  void clear() noexcept { _vec.clear(); }

  auto pop() noexcept -> Option<u8> { return _vec.pop(); }

  auto push(u8 c) noexcept -> void { _vec.push(c); }

  auto write(Slice<const u8> buf) -> usize {
    _vec.append_elements(buf);
    return buf.len();
  }

  auto push_str(Str s) -> void { _vec.append_elements(s.as_bytes()); }

  template <class P>
  auto contains(const P& pat) const noexcept -> bool {
    return this->as_str().contains(pat);
  }

  template <class P>
  auto find(const P& pat) const noexcept -> Option<usize> {
    return this->as_str().find(pat);
  }

  template <class P>
  auto rfind(const P& pat) const noexcept -> Option<usize> {
    return this->as_str().rfind(pat);
  }

  template <class P>
  auto starts_with(const P& pat) const noexcept -> bool {
    return this->as_str().starts_with(pat);
  }

  template <class P>
  auto ends_with(const P& pat) const noexcept -> bool {
    return this->as_str().ends_with(pat);
  }

  template <class T>
  auto parse() const -> Option<T> {
    return this->as_str().parse<T>();
  }

  template<class Out>
  void fmt(fmt::Formatter<Out>& formatter) const {
    formatter.pad(this->as_str());
  }
};

template <>
struct IntoString<Str> {
  const Str& _self;
  auto into_string() const -> String {
    auto res = String::with_capacity(_self.len());
    res._vec.append_elements(_self._inner);
    return res;
  }
};

template <usize N>
struct IntoString<const char8_t (&)[N]> {
  const u8 (&_self)[N];
  auto into_string() const -> String {
    return IntoString<Str>(_self).into_string();
  }
};

template <usize N>
struct IntoString<const char (&)[N]> {
  const char (&_self)[N];
  auto into_string() const -> String {
    return IntoString<Str>{_self}.into_string();
  }
};

template <class... T>
auto format(Str fmts, const T&... vals) noexcept -> string::String {
  auto res = string::String();
  fmt::Formatter{res}.write_fmt(fmt::Args{fmts, vals...});
  return res;
}

}  // namespace rc::string

namespace rc {
using string::String;
}
