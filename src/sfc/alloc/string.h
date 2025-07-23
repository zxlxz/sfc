#pragma once

#include "sfc/alloc/vec.h"

namespace sfc::string {

class [[nodiscard]] String {
  vec::Vec<char> _vec = {};

 public:
  String() noexcept = default;

  ~String() = default;

  String(String&&) noexcept = default;

  auto operator=(String&&) noexcept -> String& = default;

  static auto with_capacity(usize capacity) -> String {
    auto res = String{};
    res._vec = res._vec.with_capacity(capacity);
    return res;
  }

  static auto from(Str s) -> String {
    auto res = String{};
    res.push_str(s);
    return res;
  }

  static auto from_cstr(const char* s) -> String {
    auto res = String{};
    res.push_str(Str::from_cstr(s));
    return res;
  }

  operator str::Str() const {
    return Str{_vec.as_ptr(), _vec.len()};
  }

  auto as_ptr() const -> const char* {
    return _vec.as_ptr();
  }

  auto as_mut_ptr() -> char* {
    return _vec.as_mut_ptr();
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

  auto as_mut_vec() -> vec::Vec<u8>& {
    return reinterpret_cast<vec::Vec<u8>&>(_vec);
  }

 public:
  auto operator[](usize idx) const -> char {
    return _vec[idx];
  }

  auto operator[](usize idx) -> char& {
    return _vec[idx];
  }

  auto operator[](Range ids) const -> Str {
    return Str::from_chars(_vec[ids]);
  }

  auto iter() const {
    return _vec.iter();
  }

  auto iter_mut() {
    return _vec.iter_mut();
  }

 public:
  auto operator==(const auto& other) const -> bool {
    if constexpr (__is_convertible_to(decltype(other), const String&)) {
      return this->as_str() == other.as_str();
    }
    if constexpr (__is_convertible_to(decltype(other), Str)) {
      return this->as_str() == other;
    }
    return this->as_str() == other;
  }

 public:
  auto pop() -> Option<char>;
  void push(char c);
  void push_str(Str s);
  void write_str(Str s);

  void insert(usize idx, char ch);
  void insert_str(usize idx, Str str);

  void reserve(usize additional);
  void truncate(usize len);
  auto remove(usize idx) -> char;

  void drain(Range range);

  void clear();

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
  void fmt(auto& f) const {
    f.pad(this->as_str());
  }
};

auto format(Str fmts, const auto&... args) -> String {
  auto s = String{};
  fmt::write(s, fmts, args...);
  return s;
}

}  // namespace sfc::string

namespace sfc::panicking {

inline RawStr::operator sfc::str::Str() const {
  return sfc::str::Str{_ptr, _len};
}

[[noreturn]] void panic_str(Str msg);

[[noreturn]] void panic_imp(Location loc, const auto&... args) {
  auto s = string::String{};
  fmt::write(s, args...);
  fmt::write(s, "\n >: {}:{}", loc.file, loc.line);
  panicking::panic_str(s.as_str());
}

}  // namespace sfc::panicking
