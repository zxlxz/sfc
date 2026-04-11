#pragma once

#include "sfc/core.h"

namespace sfc::boxed {

template <class T>
class [[nodiscard]] Box {
  T* _ptr = nullptr;

 public:
  Box() noexcept = default;

  explicit Box(T val) noexcept : _ptr{new T{static_cast<T&&>(val)}} {}

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

  template <class B>
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
class [[nodiscard]] Box<T[]> {
  Slice<T> _inn;

 public:
  Box() noexcept = default;

  ~Box() {
    if (!_inn._ptr) return;
    delete[] _inn._ptr;
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

  auto ptr() const noexcept -> T* {
    return _inn._ptr;
  }

  auto len() const noexcept -> usize {
    return _inn._len;
  }

  auto as_slice() const noexcept -> Slice<T> {
    return Slice<T>{_inn._ptr, _inn._len};
  }
};

template <class R, class... T>
class [[nodiscard]] Box<R(T...)> {
  using dtor_t = void (*)(void*);
  using call_t = R (*)(void*, T&&...);

  struct Meta {
    dtor_t _dtor = nullptr;
    call_t _call = nullptr;

    template <class X>
    static constexpr auto of() -> const Meta& {
      static const auto res = Meta{
          [](void* p) { delete static_cast<X*>(p); },
          [](void* p, T&&... t) { return (*static_cast<X*>(p))(static_cast<T&&>(t)...); },
      };
      return res;
    }
  };

  void* _data{nullptr};
  const Meta* _meta{nullptr};

 public:
  Box() noexcept = default;

  explicit Box(auto fn) noexcept {
    using Fn = decltype(fn);
    this->_data = new auto{mem::move(fn)};
    this->_meta = &Meta::template of<Fn>();
  }

  ~Box() noexcept {
    if (!_data) return;
    (_meta->_dtor)(_data);
  }

  Box(Box&& other) noexcept : _data{mem::take(other._data)}, _meta{mem::take(other._meta)} {}

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      mem::swap(_data, other._data);
      mem::swap(_meta, other._meta);
    }
    return *this;
  }

  auto ptr() const noexcept -> void* {
    return _data;
  }

  auto operator()(T... args) -> R {
    return (_meta->_call)(_data, static_cast<T&&>(args)...);
  }
};

}  // namespace sfc::boxed

namespace sfc {
using boxed::Box;
}  // namespace sfc
