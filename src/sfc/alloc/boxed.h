#pragma once

#include "sfc/core.h"

namespace sfc::boxed {

template <class T>
class [[nodiscard]] Box;

template <class T>
class Box {
  T* _ptr = nullptr;

 public:
  Box() noexcept = default;

  ~Box() {
    if (_ptr) {
      delete _ptr;
    }
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

  template <class... U>
  static auto xnew(U&&... args) -> Box {
    auto res = Box{};
    res._ptr = new T{static_cast<U&&>(args)...};
    return res;
  }

  auto ptr() const noexcept -> T* {
    return _ptr;
  }

  auto operator->() const noexcept -> const T* {
    return _ptr;
  }

  auto operator->() noexcept -> T* {
    return _ptr;
  }

  auto operator*() const noexcept -> const T& {
    return *_ptr;
  }

  auto operator*() noexcept -> T& {
    return *_ptr;
  }

  auto into_raw() && noexcept -> T* {
    return mem::take(_ptr);
  }

  template <trait::polymorphic_ B>
  auto cast() && noexcept -> Box<B> {
    const auto p = static_cast<B*>(mem::take(_ptr));
    return Box<B>::from_raw(p);
  }

 public:
  // trait: fmt::Display
  void fmt(auto& f) const {
    if (_ptr == nullptr) {
      f.write_str("Box()");
    } else {
      f.write_fmt("Box({})", *_ptr);
    }
  }
};

template <class T>
class Box<T[]> {
  Slice<T> _inn;

 public:
  Box() noexcept = default;

  ~Box() {
    if (_inn._ptr != nullptr) {
      delete[] _inn._ptr;
    }
  }

  Box(Box&& other) noexcept : _inn{mem::take(other._inn)} {}

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      mem::swap(_inn, other._inn);
    }
    return *this;
  }

  static auto from_raw(Slice<T> raw) -> Box {
    auto res = Box{};
    res._inn = raw;
    return res;
  }

  static auto xnew_uninit_slice(usize len) -> Box {
    const auto p = static_cast<T*>(::operator new[](sizeof(T) * len));
    return Box::from_raw(Slice<T>{p, len});
  }

  static auto xnew(Slice<const T> x) -> Box {
    auto res = Box::xnew_uninit_slice(x._len);
    ptr::uninit_copy(x._ptr, res._inn._ptr, x._len);
    return res;
  }

  auto ptr() const noexcept -> T* {
    return _inn._ptr;
  }

  auto len() const noexcept -> usize {
    return _inn._len;
  }
};

template <class R, class... T>
class Box<R(T...)> {
 public:
  using dtor_t = void (*)(void*);
  using call_t = R (*)(void*, T&&...);
  struct Meta {
    dtor_t _dtor = nullptr;
    call_t _call = nullptr;
  };

  void* _data{nullptr};
  const Meta* _meta{nullptr};

 public:
  Box() noexcept = default;

  ~Box() noexcept {
    _data ? (_meta->_dtor)(_data) : void();
  }

  Box(Box&& other) noexcept : _data{mem::take(other._data)}, _meta{mem::take(other._meta)} {}

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      mem::swap(_data, other._data);
      mem::swap(_meta, other._meta);
    }
    return *this;
  }

  template <class X>
  static auto xnew(X x) noexcept -> Box {
    static const auto meta = Meta{
        [](void* p) { delete static_cast<X*>(p); },
        [](void* p, T&&... t) { return (*static_cast<X*>(p))(static_cast<T&&>(t)...); },
    };

    auto res = Box{};
    res._data = new auto{mem::move(x)};
    res._meta = &meta;
    return res;
  }

  auto ptr() const noexcept -> void* {
    return _data;
  }

  auto operator()(T... args) -> R {
    return (_meta->_call)(_data, static_cast<T&&>(args)...);
  }
};

template <class B>
auto box(B&& b) -> Box<B> {
  return Box<B>::xnew(static_cast<B&&>(b));
}

}  // namespace sfc::boxed

namespace sfc::option {

template <class... T>
class Inner<boxed::Box<T...>> {
  using Box = boxed::Box<T...>;
  Box _val{};

 public:
  Inner(none_t) noexcept {}
  Inner(some_t, auto&&... args) noexcept : _val{static_cast<decltype(args)&&>(args)...} {}

  Inner(Inner&&) noexcept = default;
  Inner& operator=(Inner&&) noexcept = default;

  explicit operator bool() const noexcept {
    if constexpr (requires { _val._ptr; }) {
      return _val._ptr != nullptr;
    } else {
      return _val._data != nullptr;
    }
  }

  auto operator*() const noexcept -> const Box& {
    return _val;
  }

  auto operator*() noexcept -> Box& {
    return _val;
  }
};
}  // namespace sfc::option

namespace sfc {
using boxed::Box;
using boxed::box;
}  // namespace sfc
