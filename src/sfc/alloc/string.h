#pragma once

#include "vec.h"

namespace sfc::string {

class [[nodiscard]] String {
  Vec<char> _vec = {};

 public:
  String() noexcept = default;

  ~String() = default;

  String(String&&) noexcept = default;

  String& operator=(String&&) noexcept = default;

  static auto from(Str s) -> String {
    auto res = String{};
    res.push_str(s);
    return res;
  }

  auto as_ptr() const -> const char* {
    return _vec.as_ptr();
  }

  auto as_mut_ptr() -> char* {
    return _vec.as_mut_ptr();
  }

  auto c_str() const -> const char* {
    const auto p = _vec.as_ptr();
    return reinterpret_cast<const char*>(p);
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

  auto as_mut_vec() -> Vec<char>& {
    return _vec;
  }

  explicit operator bool() const {
    return bool(_vec);
  }

  operator Str() const noexcept {
    return this->as_str();
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
    return this->as_str().operator==(other);
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

[[noreturn]] void panic_str(Location loc, Str msg);

[[noreturn]] void panic_fmt(LocationFmt info, const auto&... args) {
  const auto fmt = Str{info.fmt._ptr, info.fmt._len};
  const auto msg = string::format(fmt, args...);
  panic_str(info.loc, msg.as_str());
}

}  // namespace sfc::panicking

namespace sfc {
using string::String;
}  // namespace sfc
