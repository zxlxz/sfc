#pragma once

#include "sfc/alloc/list.h"
#include "sfc/core/chr.h"

namespace sfc::string {

using slice::Range;

template <class A = alloc::Global>
class [[nodiscard]] String {
  List<u8, A> _buf;

  explicit String(List<u8, A> buf) noexcept : _buf{mem::move(buf)} {}

 public:
  static auto new_() -> String {
    return String{List<u8, A>::new_()};
  }

  static auto with_capacity(usize capacity) -> String {
    return String{List<u8, A>::with_capacity(capacity)};
  }

  static auto from(Str s) -> String {
    auto res = String::with_capacity(s._len);
    res.push_str(s);
    return res;
  }

  static auto from_utf8(List<u8, A> buf) -> String {
    return String{mem::move(buf)};
  }

  static auto from_utf16(Slice<const u16> buf) -> String {
    auto res = String::with_capacity(buf.len());
    chr::decode_utf16(buf).for_each([&](char32_t c) { res.push(c); });
    return res;
  }

 public:
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

  auto as_str() const noexcept -> Str {
    return Str::from_utf8(_buf.as_slice());
  }

  auto as_bytes() const noexcept -> Slice<const u8> {
    return _buf.as_slice();
  }

  auto as_mut_list() noexcept -> List<u8, A>& {
    return _buf;
  }

  auto operator[](usize idx) const noexcept -> u8 {
    return _buf[idx];
  }

  auto operator[](Range ids) const noexcept -> Str {
    return Str::from_utf8(_buf[ids]);
  }

 public:
  auto pop() noexcept -> Option<char32_t> {
    auto chars = this->as_str().chars();
    const auto ret = chars.next_back();
    _buf.set_len(chars._buf._len);
    return ret;
  }

  void push(char32_t c) noexcept {
    u8 buf[4] = {};
    const auto s = chr::encode_utf8(c, buf);
    this->push_str(s);
  }

  void push_str(Str s) noexcept {
    _buf.extend_from_slice(s.as_bytes());
  }

  void reserve(usize amt) noexcept {
    _buf.reserve(amt);
  }

  void truncate(usize len) noexcept {
    _buf.truncate(len);
  }

  void clear() noexcept {
    _buf.clear();
  }

  void extend(auto iter) {
    _buf.extend(mem::move(iter));
  }

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

  template <class T>
  auto parse() const -> Option<T> {
    return this->as_str().template parse<T>();
  }

 public:
  // trait: ops::Eq
  auto operator==(const String& other) const noexcept -> bool {
    return this->as_str() == other.as_str();
  }

  // trait: ops::Eq
  auto operator==(Str other) const noexcept -> bool {
    return this->as_str() == other;
  }

  // trait: Deref<Str>
  operator Str() const noexcept {
    return this->as_str();
  }

  // trait: Deref<Str>
  auto operator*() const noexcept -> Str {
    return this->as_str();
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    return this->as_str().fmt(f);
  }

  // trait: Clone
  auto clone() const noexcept -> String {
    return String{_buf.clone()};
  }

  // trait: fmt::Write
  void write_str(Str s) noexcept {
    this->push_str(s);
  }

  // trait: serde::Serialize
  void serialize(auto& s) const {
    s.serialize_str(this->as_str());
  }
};

template <class A = alloc::Global>
auto format(const fmt::Fmts& fmts, const auto&... args) -> String<A> {
  auto out = String<A>::new_();
  fmt::write(out, fmts, args...);
  return out;
}

}  // namespace sfc::string

namespace sfc {
using String = string::String<>;
}  // namespace sfc
