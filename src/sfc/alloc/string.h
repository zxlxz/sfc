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

  static auto from_utf8(Vec<u8> vec) -> String {
    auto res = String{};
    res._vec = mem::move(vec);
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

  auto operator[](ops::Range ids) const noexcept -> Str {
    const auto v = _vec[ids];
    return Str::from_utf8(v);
  }

 public:
  auto pop() noexcept -> Option<char32_t>;

  void push(char32_t c) noexcept;
  void push_str(Str s) noexcept;

  void reserve(usize amt) noexcept;
  void truncate(usize len) noexcept;
  void clear() noexcept;

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
    this->as_str().fmt(f);
  }

  // trait: fmt::Write
  void write_str(Str s) {
    this->push_str(s);
  }

  // trait: serde::Serialize
  void serialize(auto& s) const {
    s.serialize_str(this->as_str());
  }
};

}  // namespace sfc::string

namespace sfc::fmt {
template <class... T>
auto format(fmt::fmts_t<T...> fmts, const T&... args) -> string::String {
  auto buf = string::String{};
  Fmter{buf}.write_fmt(fmts, args...);
  return buf;
}
}  // namespace sfc::fmt

namespace sfc {
using string::String;
}  // namespace sfc
