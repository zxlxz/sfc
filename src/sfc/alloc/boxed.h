#pragma once

#include "sfc/core.h"

namespace sfc::boxed {

template <class T>
class [[nodiscard]] Box {
  T* _ptr{nullptr};

 public:
  Box() noexcept = default;

  ~Box() noexcept {
    if (!_ptr) return;
    delete _ptr;
  }

  Box(Box&& other) noexcept : _ptr{mem::take(other._ptr)} {}

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
    }
    return *this;
  }

  static auto from_raw(T* ptr) noexcept -> Box {
    auto res = Box{};
    res._ptr = ptr;
    return res;
  }

  static auto new_(auto&&... args) -> Box {
    auto res = Box{};
    res._ptr = new T{(decltype(args)&&)(args)...};
    return res;
  }

 public:
  auto is_null() const noexcept -> bool {
    return _ptr == nullptr;
  }

  auto as_ptr() const noexcept -> T* {
    return _ptr;
  }

  auto into_raw() && noexcept -> T* {
    return mem::take(_ptr);
  }

 public:
  // trait: Deref<const T*>
  auto operator->() const noexcept -> const T* {
    return _ptr;
  }

  // trait: Deref<T*>
  auto operator->() noexcept -> T* {
    return _ptr;
  }

  // trait: Deref<const T&>
  auto operator*() const noexcept -> const T& {
    return *_ptr;
  }

  // trait: Deref<T&>
  auto operator*() noexcept -> T& {
    return *_ptr;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_ptr == nullptr) {
      f.write_str("Box()");
    } else {
      f.write_fmt("Box({})", *_ptr);
    }
  }
};

template <class R, class... T>
class [[nodiscard]] Box<R(T...)> {
  using Inn = ops::Fn<R(T...)>;
  Inn _inn;
  void (*_drop)(void*);

 public:
  Box() noexcept : _inn{nullptr, nullptr}, _drop{nullptr} {}

  ~Box() noexcept {
    if (!_drop || !_inn._self) return;
    (_drop)(_inn._self);
  }

  Box(Box&& other) noexcept : _inn{other._inn}, _drop{other._drop} {
    other._inn._self = nullptr;
    other._drop = nullptr;
  }

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      mem::swap(_inn, other._inn);
      mem::swap(_drop, other._drop);
    }
    return *this;
  }

  template <class Impl>
  static auto new_(Impl impl) noexcept -> Box {
    auto res = Box{};
    res._inn = Inn(*new Impl{mem::move(impl)});
    res._drop = [](void* p) { delete (Impl*)(p); };
    return res;
  }

 public:
  auto operator()(T... args) -> R {
    return _inn((T&&)(args)...);
  }
};

template <class T>
auto box(T val) -> Box<T> {
  return Box<T>::new_(mem::move(val));
}

}  // namespace sfc::boxed

namespace sfc {
using boxed::Box;
}  // namespace sfc
