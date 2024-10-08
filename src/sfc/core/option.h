#pragma once

#include "panicking.h"
#include "ptr.h"
#include "trait.h"

namespace sfc::option {

struct None {};

namespace detail {

template <class T>
class Option {
  union Imp {
    T _val;

    [[sfc_inline]] Imp() noexcept {}
    [[sfc_inline]] Imp(T&& x) noexcept : _val{static_cast<T&&>(x)} {}
    [[sfc_inline]] ~Imp() noexcept {}
  };

  bool _tag;
  Imp _imp;

 public:
  [[sfc_inline]] Option() noexcept : _tag{false}, _imp{} {}

  [[sfc_inline]] Option(None) noexcept : _tag{false}, _imp{} {}

  [[sfc_inline]] Option(T val) noexcept : _tag{true}, _imp{static_cast<T&&>(val)} {}

  [[sfc_inline]] ~Option() {
    if (_tag) {
      _imp._val.~T();
      _tag = false;
    }
  }

  [[sfc_inline]] ~Option()
    requires(trait::TvDtor<T>)
  = default;

  [[sfc_inline]] Option(const Option& other) : _tag{other._tag} {
    if (_tag) {
      new (mem::inplace_t{}, &_imp._val) T{other._imp._val};
    }
  }

  [[sfc_inline]] Option(Option&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (mem::inplace_t{}, &_imp._val) T{static_cast<T&&>(other._imp._val)};
    }
  }

  [[sfc_inline]] auto operator=(const Option& other) noexcept -> Option& {
    if (_tag == other._tag) {
      if (_tag) _imp._val = other._imp._val;
    } else {
      _tag ? mem::drop(_imp._val) : ptr::write(&_imp._val, other._imp._val);
      _tag = other._tag;
    }
    return *this;
  }

  [[sfc_inline]] auto operator=(Option&& other) noexcept -> Option& {
    if (_tag == other._tag) {
      if (_tag) _imp._val = static_cast<T&&>(other._imp._val);
    } else {
      _tag ? mem::drop(_imp._val) : ptr::write(&_imp._val, static_cast<T&&>(other._imp._val));
      _tag = other._tag;
    }
    return *this;
  }

  [[sfc_inline]] operator bool() const {
    return _tag;
  }

  [[sfc_inline]] auto get_unchecked() const -> const T& {
    return _imp._val;
  }

  [[sfc_inline]] auto get_unchecked_mut() -> T& {
    return _imp._val;
  }

  [[sfc_inline]] auto unwrap_unchecked() -> T {
    return static_cast<T&&>(_imp._val);
  }
};

template <class T>
class Option<T&> {
  T* _ptr = nullptr;

 public:
  [[sfc_inline]] Option() = default;

  [[sfc_inline]] ~Option() {}

  [[sfc_inline]] Option(T& val) : _ptr{&val} {}

  [[sfc_inline]] operator bool() const {
    return _ptr != nullptr;
  }

  [[sfc_inline]] auto get_unchecked() const -> const T& {
    return *_ptr;
  }

  [[sfc_inline]] auto get_unchecked_mut() -> T& {
    return *_ptr;
  }

  [[sfc_inline]] auto unwrap_unchecked() -> T& {
    return *_ptr;
  }
};

template <class T>
concept iBool = requires(const T& val) { val.operator bool(); };

template <iBool T>
class Option<T> {
  T _val{};

 public:
  [[sfc_inline]] Option() = default;

  [[sfc_inline]] Option(None) : _val{} {}

  [[sfc_inline]] Option(T val) : _val{static_cast<T&&>(val)} {}

  [[sfc_inline]] operator bool() const {
    return static_cast<bool>(_val);
  }

  [[sfc_inline]] auto get_unchecked() const -> const T& {
    return _val;
  }

  [[sfc_inline]] auto get_unchecked_mut() -> T& {
    return _val;
  }

  [[sfc_inline]] auto unwrap_unchecked() -> T {
    return static_cast<T&&>(_val);
  }
};

}  // namespace detail

template <class T>
class Option : detail::Option<T> {
  using Imp = detail::Option<T>;

 public:
  using Imp::Imp;

  [[sfc_inline]] Option() = default;

  [[sfc_inline]] ~Option() = default;

  [[sfc_inline]] Option(const Option&) noexcept = default;

  [[sfc_inline]] Option(Option&&) noexcept = default;

  [[sfc_inline]] Option& operator=(Option&&) noexcept = default;

  using Imp::operator bool;

  using Imp::get_unchecked;
  using Imp::get_unchecked_mut;

  using Imp::unwrap_unchecked;

  auto operator*() const -> const T& {
    assert_fmt(*this, "Option::operator*: None.");
    return Imp::get_unchecked();
  }

  auto operator*() -> T& {
    assert_fmt(*this, "Option::operator*: None.");
    return Imp::get_unchecked_mut();
  }

  auto unwrap() && -> T {
    assert_fmt(*this, "Option::unwrap: None.");
    return static_cast<T&&>(Imp::get_unchecked_mut());
  }

  auto unwrap() const& -> T {
    assert_fmt(bool(*this), "Option::unwrap: None.");
    return Imp::get_unchecked();
  }

  auto unwrap_or(T default_val) && -> T {
    if (!*this) {
      return default_val;
    }

    return static_cast<T&&>(Imp::get_unchecked_mut());
  }

  auto unwrap_or(T default_val) const& -> T {
    if (!*this) {
      return default_val;
    }

    return Imp::get_unchecked();
  }

  auto expect(const auto&... msg) && -> T {
    if (!*this) {
      panic_fmt(msg...);
    }
    return static_cast<T&&>(Imp::get_unchecked_mut());
  }

  auto operator==(const Option& other) const -> bool {
    if (!*this) return !other;
    if (!other) return false;

    return Imp::get_unchecked() == other.get_unchecked();
  }

  auto operator==(const T& rhs) const -> bool {
    if (!*this) {
      return false;
    }
    return Imp::get_unchecked() == rhs;
  }

  auto map(auto&& pred) && -> Option<decltype(pred(declval<T>()))> {
    if (!*this) {
      return {};
    }
    return pred(static_cast<T&&>(Imp::get_unchecked_mut()));
  }

  auto map(auto pred) const -> Option<decltype(pred(declval<const T&>()))> {
    if (!*this) {
      return {};
    }
    return pred(Imp::get_unchecked());
  }

  void fmt(auto& f) const {
    if (!*this) {
      f.write_str("None()");
    } else {
      f.write_fmt("Some({})", Imp::get_unchecked());
    }
  }
};

}  // namespace sfc::option

namespace sfc {
using option::Option;
}  // namespace sfc
