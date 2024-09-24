#pragma once

#include "sfc/core/chr.h"
#include "sfc/core/slice.h"

namespace sfc::str {

struct Str : private slice::Slice<const char> {
  using Inn = slice::Slice<const char>;
  using Inn::_len;
  using Inn::_ptr;

  [[sfc_inline]] Str(const char* p, usize n) noexcept : Inn{p, n} {}

 public:
  [[sfc_inline]] Str() = default;

  [[sfc_inline]] Str(const char* s) noexcept : Inn{s, s == nullptr ? 0 : __builtin_strlen(s)} {}

  [[sfc_inline]] static auto from_u8(Slice<const char> v) -> Str {
    const auto len = v ? __builtin_strlen(v._ptr) : 0U;
    return Str{v._ptr, len};
  }

  [[sfc_inline]] static auto from_u8_unchecked(Slice<const char> v) -> Str {
    return Str{v._ptr, v._len};
  }

  using Inn::as_ptr;
  using Inn::is_empty;
  using Inn::len;
  using Inn::operator bool;

  [[sfc_inline]] auto as_chars() const -> slice::Slice<const char> {
    return {_ptr, _len};
  }

  [[sfc_inline]] auto as_bytes() const -> slice::Slice<const u8> {
    return {static_cast<const u8*>(static_cast<const void*>(_ptr)), _len};
  }

 public:
  [[sfc_inline]] auto get_unchecked(usize idx) const -> char {
    return _ptr[idx];
  }

  [[sfc_inline]] auto get_unchecked(Range<> ids) const -> Str {
    return {_ptr + ids._start, ids.len()};
  }

  [[sfc_inline]] auto operator[](usize idx) const -> char {
    return idx < _len ? _ptr[idx] : char(0);
  }

  [[sfc_inline]] auto operator[](Range<> ids) const -> Str {
    ids = ids % _len;
    return Str{_ptr + ids._start, ids.len()};
  }

  [[sfc_inline]] auto split_at(usize mid) const -> tuple::Tuple<Str, Str> {
    const auto pos = cmp::min(mid, _len);
    const auto a = Str{_ptr, pos};
    const auto b = Str{_ptr + mid, _len - mid};
    return {a, b};
  }

  [[sfc_inline]] auto iter() const -> slice::Iter<const char> {
    return {_ptr, _len};
  }

 public:
  auto operator==(Str other) const -> bool {
    if (_len != other._len) return false;

    const auto ret = __builtin_memcmp(_ptr, other._ptr, _len);
    return ret == 0;
  }

 public:
  auto find(auto&& p) const -> option::Option<usize>;
  auto rfind(auto&& p) const -> option::Option<usize>;

  auto contains(auto&& p) const -> bool;
  auto starts_with(auto&& p) const -> bool;
  auto ends_with(auto&& p) const -> bool;

  auto trim_start_matches(auto&& p) const -> Str;
  auto trim_end_matches(auto&& p) const -> Str;
  auto trim_matches(auto&& p) const -> Str;

  auto trim_start() const -> Str {
    return this->trim_start_matches(chr::is_whitespace);
  }

  auto trim_end() const -> Str {
    return this->trim_end_matches(chr::is_whitespace);
  }

  auto trim() const -> Str {
    return this->trim_matches(chr::is_whitespace);
  }

 public:
  template <class T>
  auto parse() const -> option::Option<T>;

  template <class T>

  auto parse() const -> option::Option<T>
    requires(requires() { T::from_str(*this); })
  {
    return T::from_str(*this);
  }

  void fmt(auto& f) const {
    f.pad(*this);
  }
};

}  // namespace sfc::str
