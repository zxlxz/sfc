#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::string {

class [[nodiscard]] String {
  Vec<u8> _vec = {};

 public:
  static auto with_capacity(usize capacity) noexcept -> String {
    auto res = String{};
    res.reserve(capacity);
    return res;
  }

  static auto from(Str s) noexcept -> String {
    auto res = String{};
    res.push_str(s);
    return res;
  }

  auto as_ptr() const noexcept -> const u8* {
    return _vec.as_ptr();
  }

  auto capacity() const noexcept -> usize {
    return _vec.capacity();
  }

  auto len() const noexcept -> usize {
    return _vec.len();
  }

  auto is_empty() const noexcept -> bool {
    return _vec.is_empty();
  }

  auto as_slice() const noexcept -> Slice<const u8> {
    return {_vec.as_ptr(), _vec.len()};
  }

  auto as_mut_slice() noexcept -> Slice<u8> {
    return {_vec.as_mut_ptr(), _vec.len()};
  }

  auto as_mut_vec() noexcept -> Vec<u8>& {
    return _vec;
  }

  auto as_str() const noexcept -> Str {
    return Str::from_utf8(this->as_slice());
  }

 public:
  auto operator[](usize idx) const noexcept -> u8 {
    return _vec[idx];
  }

  auto operator[](usize idx) noexcept -> u8& {
    return _vec[idx];
  }

  auto operator[](ops::Range ids) const noexcept -> Str {
    const auto v = _vec[ids];
    return Str::from_utf8(v);
  }

 public:
  auto pop() noexcept -> Option<u8> {
    return _vec.pop();
  }

  void push(u8 c) noexcept {
    _vec.push(c);
  }

  void push_str(Str s) noexcept {
    _vec.extend_from_slice(s.as_bytes());
  }

  void reserve(usize amt) noexcept {
    _vec.reserve(amt);
  }

  void truncate(usize len) noexcept {
    _vec.truncate(len);
  }

  void clear() noexcept {
    _vec.clear();
  }

  void insert(usize idx, u8 ch) noexcept {
    _vec.insert(idx, ch);
  }

  auto remove(usize idx) noexcept -> u8 {
    return _vec.remove(idx);
  }

  void drain(ops::Range ids) noexcept {
    _vec.drain(ids);
  }

  void insert_str(usize idx, Str str) noexcept {
    const auto len = _vec.len();
    if (idx > len) {
      idx = len;
    }
    _vec.reserve(str._len);
    ptr::shift_elements_right(_vec.as_mut_ptr() + idx, len - idx, str._len);
    ptr::copy_nonoverlapping(str.as_ptr(), _vec.as_mut_ptr() + idx, str._len);
    _vec.set_len(len + str._len);
  }

 public:
  auto iter() const noexcept {
    return _vec.iter();
  }

  auto iter_mut() noexcept {
    return _vec.iter_mut();
  }

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
  // trait: Deref<Str>
  auto operator*() const noexcept -> Str {
    return Str::from_utf8(this->as_slice());
  }

  // trait: ops::Eq
  auto operator==(Str other) const noexcept -> bool {
    return this->as_str() == other;
  }

  // trait: ops::Eq
  auto operator==(const String& other) const noexcept -> bool {
    return this->as_str() == other.as_str();
  }

  // trait: Clone
  auto clone() const noexcept -> String {
    return String::from(this->as_str());
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    f.pad(this->as_str());
  }

  // trait: fmt::Write
  void write_str(Str s) {
    _vec.extend_from_slice(s.as_bytes());
  }

  // trait: serde::Serialize
  void serialize(auto& s) const {
    s.serialize_str(this->as_str());
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

namespace sfc::panicking {

[[noreturn]] void panic_str(Location loc, Str msg) noexcept;

[[noreturn]] void panic_fmt(Location loc, const auto&... args) noexcept {
  if constexpr (sizeof...(args) == 0) {
    panicking::panic_str(loc, "panic occurred");
  } else if constexpr (sizeof...(args) == 1) {
    panicking::panic_str(loc, args...);
  } else {
    const auto msg = string::format(args...);
    panicking::panic_str(loc, msg.as_str());
  }
}

}  // namespace sfc::panicking

namespace sfc {
using string::String;
}  // namespace sfc
