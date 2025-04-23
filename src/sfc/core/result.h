#pragma once

#include "sfc/core/option.h"

namespace sfc::result {

namespace detail {

template <class T, class E>
class Result {
 protected:
  union Imp {
    T _ok;
    E _err;

    Imp(T&& ok) : _ok{static_cast<T&&>(ok)} {}
    Imp(E&& err) : _err{static_cast<E&&>(err)} {}
    ~Imp() {}
  };
  bool _tag;
  Imp  _imp;

 public:
  Result(T ok) : _tag{true}, _imp{static_cast<T&&>(ok)} {}

  Result(E err) : _tag{false}, _imp{static_cast<E&&>(err)} {}

  ~Result() {
    if (_tag) {
      _imp._ok.~T();
    } else {
      _imp._err.~E();
    }
  }

  Result(Result&& other) noexcept : _tag{other._tag} {
    if (_tag) {
      new (mem::inplace_t{}, &_imp._ok) T{static_cast<T&&>(other._imp._ok)};
    } else {
      new (mem::inplace_t{}, &_imp._err) T{static_cast<T&&>(other._imp._err)};
    }
  }

  Result& operator=(const Result& other) {
    if (_tag == other._tag) {
      _tag ? _imp._ok = other._imp._ok : _imp._err = other._imp._err;
    } else {
      _tag ? _imp._ok.~T() : _imp._err.~E();
      _tag = other._tag;
      _tag ? new (mem::inplace_t{}, &_imp._ok) T{other._imp._ok}
           : new (mem::inplace_t{}, &_imp._err) E{other._imp._err};
    }
    return *this;
  }

  Result& operator=(Result&& other) noexcept {
    if (_tag == other._tag) {
      _tag ? _imp._ok = static_cast<T&&>(other._imp._ok)
           : _imp._err = static_cast<E&&>(other._imp._err);
    } else {
      _tag ? _imp._ok.~T() : _imp._err.~E();
      _tag = other._tag;
      _tag ? new (mem::inplace_t{}, &_imp._ok) T{static_cast<T&&>(other._imp._ok)}
           : new (mem::inplace_t{}, &_imp._err) E{static_cast<E&&>(other._imp._err)};
    }
    return *this;
  }

  explicit operator bool() const {
    return _tag;
  }

  auto get_ok_unchecked() const -> const T& {
    return _imp._ok;
  }

  auto get_ok_unchecked_mut() -> T& {
    return _imp._ok;
  }

  auto get_err_unchecked() const -> const E& {
    return _imp._err;
  }

  auto get_err_unchecked_mut() -> E& {
    return _imp._err;
  }
};

}  // namespace detail

template <class T, class E>
class Result : detail::Result<T, E> {
  using Imp = detail::Result<T, E>;
  using Imp::_imp;
  using Imp::_tag;

 public:
  using Imp::Imp;

  ~Result() = default;

  Result(const Result&) = default;

  Result(Result&&) noexcept = default;

  auto operator=(Result&&) noexcept -> Result& = default;

  auto is_ok() const -> bool {
    return this->_tag;
  }

  auto is_err() const -> bool {
    return !this->_tag;
  }

  using Imp::get_ok_unchecked;
  using Imp::get_ok_unchecked_mut;

  using Imp::get_err_unchecked;
  using Imp::get_err_unchecked_mut;

  auto ok() && -> Option<T> {
    if (!_tag) {
      return {};
    }
    return static_cast<T&&>(Imp::get_ok_unchecked_mut());
  }

  auto ok() const& -> Option<T> {
    if (!_tag) {
      return {};
    }
    return Imp::get_ok_unchecked();
  }

  auto err() && -> Option<E> {
    if (_tag) {
      return {};
    }
    return static_cast<E&&>(Imp::get_err_unchecked_mut());
  }

  auto err() const& -> Option<E> {
    if (_tag) {
      return {};
    }
    return Imp::get_err_unchecked();
  }

  auto unwrap() && -> T {
    assert_fmt(_tag, "Result::unwrap: Err({})", _imp._err);
    return static_cast<T&&>(Imp::get_ok_unchecked_mut());
  }

  auto unwrap() const& -> T {
    assert_fmt(_tag, "Result::unwrap: Err({})", _imp._err);
    return Imp::get_ok_unchecked();
  }

  auto unwrap_err() && -> E {
    assert_fmt(!_tag, "Option::unwrap_err: Ok({})", _imp._ok);
    return static_cast<E&&>(Imp::get_err_unchecked_mut());
  }

  auto unwrap_err() const& -> E {
    assert_fmt(!_tag, "Option::unwrap_err: Ok({})", _imp._ok);
    return Imp::get_err_unchecked();
  }
};

}  // namespace sfc::result
