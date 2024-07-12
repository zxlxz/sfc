#pragma once

#include "option.h"

namespace sfc::result {

template <class T>
concept XBool = requires(const T& x) { T::operator bool; };

namespace detail {

template <class T, class E>
class Result {
 protected:
  union Imp {
    T _ok;
    E _err;

    [[sfc_inline]] Imp(T&& ok) : _ok{static_cast<T&&>(ok)} {}
    [[sfc_inline]] Imp(E&& err) : _err{static_cast<E&&>(err)} {}
    [[sfc_inline]] ~Imp() {}
  };
  bool _tag;
  Imp _imp;

 public:
  [[sfc_inline]] Result(T ok) : _tag{true}, _imp{static_cast<T&&>(ok)} {}

  [[sfc_inline]] Result(E err) : _tag{false}, _imp{static_cast<E&&>(err)} {}

  [[sfc_inline]] ~Result() {
    if (_tag) {
      _imp._ok.~T();
    } else {
      _imp._err.~E();
    }
  }

  [[sfc_inline]] Result(Result&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (mem::inplace_t{}, &_imp._ok) T{static_cast<T&&>(other._imp._ok)};
    } else {
      new (mem::inplace_t{}, &_imp._err) T{static_cast<T&&>(other._imp._err)};
    }
  }

  [[sfc_inline]] Result& operator=(Result&& other) noexcept {
    if (_tag == other._tag) {
      _tag ? _imp._ok = static_cast<T&&>(other._imp._ok) : _imp._err = static_cast<E&&>(other._er);
    } else {
      _tag ? _imp._ok.~T() : _imp._err.~E();
      _tag = other._tag;
      _tag ? new (mem::inplace_t{}, &_imp._ok) T{static_cast<T&&>(other._imp._ok)}
           : new (mem::inplace_t{}, &_imp._err) E{static_cast<E&&>(other._imp._err)};
    }
    return *this;
  }

  [[sfc_inline]] operator bool() const {
    return _tag;
  }

  [[sfc_inline]] auto get_ok_unchecked() const -> const T& {
    return _imp._ok;
  }

  [[sfc_inline]] auto get_ok_unchecked_mut() -> T& {
    return _imp._ok;
  }

  [[sfc_inline]] auto get_err_unchecked() const -> const E& {
    return _imp._err;
  }

  [[sfc_inline]] auto get_err_unchecked_mut() -> E& {
    return _imp._err;
  }
};

template <trait::Copy T, trait::Copy E>
class Result<T, E> {
 protected:
  union Imp {
    T _ok;
    E _err;
  };
  bool _tag;
  Imp _imp;

 public:
  [[sfc_inline]] Result(T ok) : _tag{true}, _imp{._ok = ok} {}

  [[sfc_inline]] Result(E err) : _tag{false}, _imp{._err = err} {}

  [[sfc_inline]] operator bool() const {
    return _tag;
  }

  [[sfc_inline]] auto get_ok_unchecked() const -> const T& {
    return _imp._ok;
  }

  [[sfc_inline]] auto get_ok_unchecked_mut() -> T& {
    return _imp._ok;
  }

  [[sfc_inline]] auto get_err_unchecked() const -> const E& {
    return _imp._err;
  }

  [[sfc_inline]] auto get_err_unchecked_mut() -> E& {
    return _imp._err;
  }
};

}  // namespace detail

template <class T, class E>
class Result : detail::Result<T, E> {
  using Imp = detail::Result<T, E>;
  using Imp::_tag;
  using Imp::_imp;

 public:
  using Imp::Imp;

  [[sfc_inline]] ~Result() = default;

  [[sfc_inline]] Result(Result&&) noexcept = default;

  [[sfc_inline]] Result& operator=(Result&&) noexcept = default;

  [[sfc_inline]] auto is_ok() const -> bool {
    return this->_tag;
  }

  [[sfc_inline]] auto is_err() const -> bool {
    return !this->_tag;
  }

  using Imp::get_ok_unchecked;
  using Imp::get_ok_unchecked_mut;

  using Imp::get_err_unchecked;
  using Imp::get_err_unchecked_mut;

  auto ok() && -> Option<T> {
    if (!_tag) return {};
    return static_cast<T&&>(Imp::get_ok_unchecked_mut());
  }

  auto err() && -> Option<E> {
    if (_tag) return {};
    return static_cast<E&&>(Imp::get_err_unchecked_mut());
  }

  auto unwrap() && -> T {
    assert_fmt(_tag, "Result::unwrap: Err({})", _imp._err);
    return static_cast<T&&>(Imp::get_ok_unchecked_mut());
  }

  auto unwrap_err() && -> E {
    assert_fmt(!_tag, "Option::unwrap_err: Ok({})", _imp._ok);
    return static_cast<E&&>(Imp::get_err_unchecked_mut());
  }
};

template <trait::Copy T, class E>
class Result<T, E> : detail::Result<T, E> {
  using Imp = detail::Result<T, E>;
  using Imp::_tag;
  using Imp::_imp;

 public:
  using Imp::Imp;

  using Imp::get_ok_unchecked;
  using Imp::get_ok_unchecked_mut;

  using Imp::get_err_unchecked;
  using Imp::get_err_unchecked_mut;

  [[sfc_inline]] auto is_ok() const -> bool {
    return _tag;
  }

  [[sfc_inline]] auto is_err() const -> bool {
    return !_tag;
  }

  auto ok() const -> Option<T> {
    if (!_tag) return {};
    return Imp::get_ok_unchecked();
  }

  auto err() const -> Option<E> {
    if (_tag) return {};
    return Imp::get_err_unchecked();
  }

  auto unwrap() const -> T {
    assert_fmt(_tag, "Result::unwrap: Err({})", _imp._err);
    return Imp::get_ok_unchecked();
  }

  auto unwrap_err() const -> E {
    assert_fmt(!_tag, "Option::unwrap_err: Ok({})", _imp._ok);
    return Imp::get_err_unchecked();
  }
};

}  // namespace sfc::result
