#pragma once

#include "sfc/core/panicking.h"
#include "sfc/core/ptr.h"

namespace sfc::option {

enum class Tag : u8 { None, Some };

namespace detail {

template <class T>
class Option {
  Tag _tag = Tag::None;
  union {
    T _val;
  };

 public:
  explicit Option() noexcept {}

  explicit Option(T&& val) noexcept : _tag{Tag::Some}, _val{static_cast<T&&>(val)} {}

  ~Option() {
    if (_tag == Tag::Some) {
      _val.~T();
    }
  }

  Option(const Option& other) : _tag{other._tag} {
    if (_tag == Tag::Some) {
      new (&_val) T{other._val};
    }
  }

  Option(Option&& other) noexcept : _tag{other._tag} {
    if (_tag == Tag::Some) {
      new (&_val) T{static_cast<T&&>(other._val)};
    }
  }

  auto operator=(const Option& other) -> Option& {
    if (this == &other) {
      return *this;
    }
    if (_tag == Tag::Some) {
      _val.~T();
    }
    if (other._tag == Tag::Some) {
      new (&_val) T{other._val};
    }
    _tag = other._tag;
    return *this;
  }

  auto operator=(Option&& other) noexcept -> Option& {
    if (this == &other) {
      return *this;
    }
    if (_tag == Tag::Some) {
      _val.~T();
    }
    if (other._tag == Tag::Some) {
      new (&_val) T{static_cast<T&&>(other._val)};
    }
    _tag = other._tag;
    return *this;
  }

  auto tag() const noexcept -> Tag {
    return _tag;
  }

  auto operator*() const noexcept -> const T& {
    return _val;
  }

  auto operator*() -> T& {
    return _val;
  }
};

template <class T>
class Option<T&> {
  T* _ptr{nullptr};

 public:
  Option() = default;

  explicit Option(T& val) noexcept : _ptr{&val} {}

  auto tag() const -> Tag {
    return _ptr != nullptr ? Tag::Some : Tag::None;
  }

  auto operator*() const -> const T& {
    return *_ptr;
  }

  auto operator*() -> T& {
    return *_ptr;
  }
};

}  // namespace detail

template <class T>
class Option {
  detail::Option<T> _inn{};

 public:
  Option() = default;
  Option(T val) noexcept : _inn{static_cast<T&&>(val)} {}

  ~Option() = default;

  Option(const Option&) noexcept = default;
  Option(Option&&) noexcept = default;

  Option& operator=(Option&&) noexcept = default;
  Option& operator=(const Option&) noexcept = default;

  operator bool() const {
    return _inn.tag() == Tag::Some;
  }

  auto is_some() const -> bool {
    return _inn.tag() == Tag::Some;
  }

  auto is_none() const -> bool {
    return _inn.tag() == Tag::None;
  }

  auto operator*() const -> const T& {
    panicking::assert(_inn.tag() == Tag::Some, "Option::operator*: deref None");
    return *_inn;
  }

  auto operator*() -> T& {
    panicking::assert(_inn.tag() == Tag::Some, "Option::operator*: deref None");
    return *_inn;
  }

  auto operator->() const {
    panicking::assert(_inn.tag() == Tag::Some, "Option::operator->: deref None");
    return &*_inn;
  }

  auto operator->() {
    panicking::assert(_inn.tag() == Tag::Some, "Option::operator->: deref None");
    return &*_inn;
  }

  auto operator==(const Option& other) const -> bool {
    if (_inn.tag() == Tag::None) {
      return other._inn.tag() == Tag::None;
    }
    if (other._inn.tag() == Tag::None) {
      return false;
    }
    return *_inn == *other._inn;
  }

  auto unwrap() && -> T {
    panicking::assert(_inn.tag() == Tag::Some, "Option::unwrap: None");
    return static_cast<T&&>(*_inn);
  }

  auto unwrap_or(T default_val) && -> T {
    return _inn.tag() == Tag::Some ? static_cast<T&&>(*_inn) : static_cast<T&&>(default_val);
  }

  auto expect(const auto&... msg) && -> T {
    panicking::assert(_inn.tag() == Tag::Some, msg...);
    return static_cast<T&&>(*_inn);
  }

  auto map(auto&& pred) && {
    using U = decltype(pred(static_cast<T&&>(*_inn)));
    if (_inn.tag() == Tag::None) {
      return Option<U>{};
    }
    return Option<U>{pred(static_cast<T&&>(*_inn))};
  }

  void fmt(auto& f) const {
    if (_inn.tag() == Tag::None) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", *_inn);
    }
  }
};

}  // namespace sfc::option
