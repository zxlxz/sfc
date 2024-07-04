#pragma once

#include "sfc/core.h"

namespace sfc::rc {

namespace detail {

template <class T>
struct RcBox {
  i32 _strong = 1;
  i32 _weak = 1;
  T _val;

  template <class... U>
  RcBox(U&&... args) : _val{static_cast<U&&>(args)...} {}

 public:
  [[sfc_inline]] auto strong() const -> i32 {
    return _strong;
  }

  [[sfc_inline]] auto weak() const -> i32 {
    return _weak;
  }

  [[sfc_inline]] auto inc_strong() -> i32 {
    return __atomic_add_fetch(&_strong, 1, __ATOMIC_RELAXED);
  }

  [[sfc_inline]] auto dec_strong() -> i32 {
    return __atomic_add_fetch(&_strong, -1, __ATOMIC_RELAXED);
  }

  [[sfc_inline]] auto inc_weak() -> i32 {
    return __atomic_add_fetch(&_weak, 1, __ATOMIC_RELAXED);
  }

  [[sfc_inline]] auto dec_weak() -> i32 {
    return __atomic_add_fetch(&_weak, -1, __ATOMIC_RELAXED);
  }
};

}  // namespace detail

template <class T>
class Weak;

template <class T>
class Rc {
  friend class Weak<T>;

  using Inn = detail::RcBox<T>;
  Inn* _ptr = nullptr;

  explicit Rc(Inn* inn) noexcept : _ptr{inn} {}

  template <class... U>
  explicit Rc(mem::inplace_t, U&&... args)
      : _ptr{Box<Inn>::xnew(static_cast<U&&>(args)...).into_raw()} {}

 public:
  Rc() noexcept = default;

  ~Rc() {
    if (!_ptr) return;

    if (_ptr->dec_strong() == 0) {
      mem::drop(*_ptr);
      if (_ptr->dec_weak() == 0) {
        alloc::Global{}.dealloc_one(_ptr);
      }
    }
  }

  Rc(Rc&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  auto operator=(Rc&& other) noexcept -> Rc& {
    auto tmp = static_cast<Rc&&>(other);
    mem::swap(_ptr, tmp._ptr);
    return *this;
  }

  template <class... U>
  static auto xnew(U&&... args) -> Rc {
    return Rc{mem::inplace_t{}, static_cast<U&&>(args)...};
  }

  auto clone() const -> Rc {
    if (_ptr) {
      _ptr->inc_strong();
    }
    return Rc{_ptr};
  }

  operator bool() const {
    return _ptr != nullptr;
  }

  auto operator->() const -> const T* {
    assert_fmt(_ptr != nullptr, "Rc::operator->: deref null");
    return &_ptr->_val;
  }

  auto operator->() -> T* {
    assert_fmt(_ptr != nullptr, "Rc::operator->: deref null");
    return &_ptr->_val;
  }

  auto operator*() const -> const T& {
    assert_fmt(_ptr != nullptr, "Rc::operator*: deref null");
    return _ptr->_val;
  }

  auto operator*() -> T& {
    assert_fmt(_ptr != nullptr, "Rc::operator*: deref null");
    return _ptr->_val;
  }

  auto weak() -> Weak<T>;
};

template <class T>
class Weak {
  detail::RcBox<T>* _ptr = nullptr;

  explicit Weak(detail::RcBox<T>* ptr) : _ptr{ptr} {}

  explicit Weak(const Rc<T>& rc) : _ptr{rc._ptr} {
    _ptr->add_weak();
  }

 public:
  Weak() noexcept = default;

  ~Weak() {
    if (!_ptr) {
      return;
    }

    if (_ptr->dec_weak() == 0) {
      alloc::Global{}.dealloc_one(_ptr);
    }
  }

  Weak(Weak&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  auto operator=(Weak&& other) noexcept -> Weak& {
    auto tmp = static_cast<Weak&&>(other);
    mem::swap(_ptr, tmp._ptr);
    return *this;
  }

  auto clone() -> Weak {
    _ptr->add_weak();
    return Weak{_ptr};
  }

  auto weak_count() const -> usize {
    if (!_ptr) {
      return 0U;
    }

    const auto res = _ptr->weak();
    return static_cast<usize>(res);
  }
};

template <class T>
auto Rc<T>::weak() -> Weak<T> {
  _ptr->add_weak();
  return {_ptr};
}

}  // namespace sfc::rc

namespace sfc {
using rc::Rc;
}  // namespace sfc
