#pragma once

#include "sfc/alloc/list.h"
#include "sfc/core/chr.h"

namespace sfc::string {

using slice::Range;

class [[nodiscard]] String {
  List<u8> _buf;

  explicit String(List<u8> buf) noexcept;

 public:
  ~String();

  String(String&&) noexcept = default;
  String& operator=(String&&) noexcept = default;

  static auto new_() -> String;
  static auto with_capacity(usize capacity) -> String;
  static auto from(Str s) -> String;
  static auto from_utf8(List<u8> buf) -> String;
  static auto from_utf16(Slice<const u16> buf) -> String;
  static auto from_wstr(Slice<const wchar_t> buf) -> String;

 public:
  auto capacity() const noexcept -> usize;
  auto len() const noexcept -> usize;
  auto is_empty() const noexcept -> bool;
  auto as_ptr() const noexcept -> const u8*;
  auto as_str() const noexcept -> Str;
  auto as_bytes() const noexcept -> Slice<const u8>;
  auto as_mut_list() noexcept -> List<u8>&;
  auto operator[](usize idx) const noexcept -> u8;
  auto operator[](Range ids) const noexcept -> Str;

 public:
  auto pop() noexcept -> Option<char32_t>;
  void push(char32_t c) noexcept;
  void push_str(Str s) noexcept;
  void reserve(usize amt) noexcept;
  void truncate(usize len) noexcept;
  void clear() noexcept;

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
  auto operator==(const String& other) const noexcept -> bool;

  // trait: ops::Eq
  auto operator==(Str other) const noexcept -> bool;

  // trait: Deref<Str>
  operator Str() const noexcept;

  // trait: Deref<Str>
  auto operator*() const noexcept -> Str;

  // trait: fmt::Display
  void fmt(auto& f) const {
    return this->as_str().fmt(f);
  }

  // trait: Clone
  auto clone() const noexcept -> String;

  // trait: fmt::Write
  void write_str(Str s) noexcept;

  // trait: serde::Serialize
  void serialize(auto& s) const {
    s.serialize_str(this->as_str());
  }

  // trait: serde::Deserialize
  template <class D>
  static auto deserialize(D& d) -> Result<String, typename D::Error> {
    const auto s = _TRY(d.deserialize_str());
    return String::from(s);
  }
};

auto format(const fmt::Fmts& fmts, const auto&... args) -> String {
  auto out = String::new_();
  fmt::write(out, fmts, args...);
  return out;
}

}  // namespace sfc::string

namespace sfc {
using String = string::String;
}  // namespace sfc
