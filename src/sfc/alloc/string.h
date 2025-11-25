#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::string {

class [[nodiscard]] String {
  Vec<u8> _vec = {};

 public:
  String() noexcept = default;
  ~String() = default;

  String(String&&) noexcept = default;
  String& operator=(String&&) noexcept = default;

  String(const String&) = delete;
  String& operator=(const String&) = delete;

  static auto with_capacity(usize capacity) -> String {
    auto res = String{};
    res.reserve(capacity);
    return res;
  }

  template <class F>
  static auto from(F&& f) -> String {
    if constexpr (requires { String{static_cast<F&&>(f)}; }) {
      return String{static_cast<F&&>(f)};
    } else if constexpr (requires { f.to_string(); }) {
      return f.to_string();
    } else if constexpr (requires { Str::from(f); }) {
      auto s = String{};
      s.push_str(Str::from(f));
      return s;
    }
  }

  operator Str() const noexcept {
    return Str{_vec.as_ptr(), _vec.len()};
  }

  auto as_ptr() const -> const u8* {
    return _vec.as_ptr();
  }

  auto capacity() const -> usize {
    return _vec.capacity();
  }

  auto len() const -> usize {
    return _vec.len();
  }

  auto as_str() const -> Str {
    return Str{_vec.as_ptr(), _vec.len()};
  }

  auto is_empty() const -> bool {
    return _vec.is_empty();
  }

  auto clone() const -> String {
    return String::from(this->as_str());
  }

  auto as_slice() const -> Slice<const u8> {
    return {_vec.as_ptr(), _vec.len()};
  }

  auto as_mut_slice() -> Slice<u8> {
    return {_vec.as_mut_ptr(), _vec.len()};
  }

  auto as_mut_vec() -> Vec<u8>& {
    return _vec;
  }

 public:
  auto operator[](usize idx) const -> u8 {
    return _vec[idx];
  }

  auto operator[](usize idx) -> u8& {
    return _vec[idx];
  }

  auto operator[](ops::Range ids) const -> Str {
    const auto v = _vec[ids];
    return Str{v._ptr, v._len};
  }

  auto iter() const {
    return _vec.iter();
  }

  auto iter_mut() {
    return _vec.iter_mut();
  }

 public:
  auto operator==(const auto& other) const -> bool {
    return this->as_str() == other;
  }

 public:
  auto pop() -> Option<u8> {
    return _vec.pop();
  }

  void push(u8 c) {
    _vec.push(c);
  }

  void push_str(Str s) {
    _vec.extend_from_slice(s.as_bytes());
  }

  void reserve(usize amt) {
    _vec.reserve(amt);
  }

  void truncate(usize len) {
    _vec.truncate(len);
  }

  void clear() {
    _vec.clear();
  }

  void insert(usize idx, u8 ch) {
    _vec.insert(idx, ch);
  }

  auto remove(usize idx) -> u8 {
    return _vec.remove(idx);
  }

  void drain(ops::Range ids) {
    _vec.drain(ids);
  }

  void insert_str(usize idx, Str str) {
    _vec.reserve(str._len);
    ptr::shift_elements_right(_vec.as_mut_ptr() + idx, _vec.len() - idx, str._len);
    ptr::copy_nonoverlapping(str.as_bytes().as_ptr(), _vec.as_mut_ptr() + idx, str._len);
    _vec.set_len(_vec.len() + str._len);
  }

 public:
  auto find(auto&& p) const -> Option<usize> {
    return this->as_str().find(p);
  }

  auto rfind(auto&& p) const -> Option<usize> {
    return this->as_str().rfind(p);
  }

  auto contains(auto&& p) const -> bool {
    return this->as_str().contains(p);
  }

  auto starts_with(auto&& p) const -> bool {
    return this->as_str().starts_with(p);
  }

  auto ends_with(auto&& p) const -> bool {
    return this->as_str().ends_with(p);
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    f.pad(this->as_str());
  }

  // trait: serde::Serialize
  void serialize(auto& s) const {
    s.serialize_str(this->as_str());
  }
};

class [[nodiscard]] CString {
  Vec<char> _vec = {};

 public:
  CString() = default;
  ~CString() = default;

  CString(CString&&) noexcept = default;
  CString& operator=(CString&&) noexcept = default;

  static auto from(Str s) -> CString {
    auto res = CString{};
    res._vec.extend_from_slice(s.as_chars());
    res._vec.push('\0');
    return res;
  }

  operator cstr_t() const {
    return _vec.as_ptr();
  }
};

auto format(Str fmts, const auto&... args) -> String {
  auto s = String{};
  fmt::Fmter{s}.write_fmt(fmts, args...);
  return s;
}

auto to_string(const auto& val) -> String {
  auto s = String{};
  fmt::write(s, {}, val);
  return s;
}

}  // namespace sfc::string

namespace sfc {
using string::String;
}  // namespace sfc

namespace sfc::panicking {

[[noreturn]] void panic_str(Str msg);

[[noreturn]] void panic_imp(Location loc, const auto&... args) {
  auto s = string::String{};
  if constexpr (sizeof...(args)) {
    fmt::write(s, args...);
  }
  fmt::write(s, "\n >: {}:{}", Str::from_cstr(loc.file), loc.line);
  panicking::panic_str(s.as_str());
}

[[noreturn]] void panic(PanicInfo info, const auto&... args) {
  panicking::panic_imp(info.loc, Str::from_cstr(info.val), args...);
}

}  // namespace sfc::panicking

namespace sfc {
using string::String;
using string::CString;
}  // namespace sfc
