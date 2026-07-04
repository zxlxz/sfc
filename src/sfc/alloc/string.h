#pragma once

#include "sfc/alloc/list.h"

namespace sfc::string {

class [[nodiscard]] String {
  using Buf = List<u8>;
  Buf _buf{};

 public:
  static auto with_capacity(usize capacity) noexcept -> String;
  static auto from(Str s) noexcept -> String;
  static auto from_buf(Buf buf) -> String;

  auto capacity() const noexcept -> usize;
  auto len() const noexcept -> usize;
  auto is_empty() const noexcept -> bool;
  auto as_ptr() const noexcept -> const u8*;
  auto as_mut_buf() noexcept -> Buf&;

  auto as_slice() const noexcept -> Slice<const u8>;
  auto as_mut_slice() noexcept -> Slice<u8>;
  auto as_str() const noexcept -> Str;
  auto operator[](ops::Range ids) const noexcept -> Str;

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
  auto operator*() const noexcept -> Str;

  // trait: ops::Eq
  auto operator==(Str other) const noexcept -> bool;

  // trait: Clone
  auto clone() const noexcept -> String;

  // trait: fmt::Write
  void write_str(Str s) noexcept;

  // trait: fmt::Display
  void fmt(fmt::Formatter& f) const;

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

namespace sfc {
using string::String;
}  // namespace sfc
