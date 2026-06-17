#pragma once

#include "sfc/alloc/list.h"

namespace sfc::string {

class [[nodiscard]] String {
  using Buf = List<u8>;
  Buf _buf = {};

 public:
  static auto with_capacity(usize capacity) noexcept -> String;
  static auto from(Str s) noexcept -> String;
  static auto from_buf(Buf buf) -> String;

  auto capacity() const noexcept -> usize {
    return _buf.capacity();
  }

  auto len() const noexcept -> usize {
    return _buf.len();
  }

  auto is_empty() const noexcept -> bool {
    return _buf.is_empty();
  }

  auto as_ptr() const noexcept -> const u8* {
    return _buf.as_ptr();
  }

  auto as_mut_buf() noexcept -> Buf& {
    return _buf;
  }

  auto as_slice() const noexcept -> Slice<const u8> {
    return _buf.as_slice();
  }

  auto as_mut_slice() noexcept -> Slice<u8> {
    return _buf.as_mut_slice();
  }

  auto as_str() const noexcept -> Str {
    return Str::from_utf8(_buf.as_slice());
  }

  auto operator[](ops::Range ids) const noexcept -> Str {
    const auto v = _buf[ids];
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
    return _buf.iter();
  }

  auto iter_mut() noexcept {
    return _buf.iter_mut();
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
    return Str::from_utf8(_buf.as_slice());
  }

  // trait: ops::Eq
  auto operator==(Str other) const noexcept -> bool {
    return this->as_str() == other;
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

auto format(const fmt::Fmts& fmts, const auto&... args) -> String {
  auto out = String{};
  fmt::write(out, fmts, args...);
  return out;
}

}  // namespace sfc::string

namespace sfc::fmt {
extern template struct Formatter<string::String>;
}

namespace sfc {
using string::String;
}  // namespace sfc
