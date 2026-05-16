#pragma once

#include "sfc/core.h"

namespace sfc::boxed {

template <class T>
class [[nodiscard]] Box {
  T* _ptr = nullptr;

 public:
  Box() noexcept = default;

  ~Box() noexcept {
    if (!_ptr) return;
    delete _ptr;
  }

  Box(Box&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  Box& operator=(Box&& other) noexcept {
    if (this != &other) {
      // just swap to other, and let other destruct old data
      mem::swap(_ptr, other._ptr);
    }
    return *this;
  }

  static auto from_raw(T* ptr) noexcept -> Box {
    auto res = Box{};
    res._ptr = ptr;
    return res;
  }

  static auto xnew(auto&&... args) -> Box {
    auto res = Box{};
    res._ptr = new T{static_cast<decltype(args)&&>(args)...};
    return res;
  }

  auto ptr() const noexcept -> T* {
    return _ptr;
  }

  auto into_raw() && noexcept -> T* {
    const auto p = _ptr;
    _ptr = nullptr;
    return p;
  }

  template <class B>
  auto cast() && noexcept -> Box<B> {
    static_assert(__has_virtual_destructor(B));
    const auto p = dynamic_cast<B*>(_ptr);
    if (p == nullptr) {
      return {};
    }
    _ptr = nullptr;
    return Box<B>::from_raw(p);
  }

 public:
  // trait: option::Nullable
  auto operator==(decltype(nullptr)) const noexcept -> bool {
    return _ptr == nullptr;
  }

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
  using dtor_t = void (*)(void*);
  using call_t = R (*)(void*, T&&...);

  struct Meta {
    dtor_t _dtor = nullptr;
    call_t _call = nullptr;

    template <class X>
    static constexpr auto of(const X&) -> const Meta& {
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

  static auto xnew(auto fn) noexcept -> Box {
    auto res = Box{};
    res._data = new auto{mem::move(fn)};
    res._meta = &Meta::of(fn);
    return res;
  }

  auto operator()(T... args) -> R {
    return (_meta->_call)(_data, static_cast<T&&>(args)...);
  }

 public:
  // trait: option::Nullable
  auto operator==(decltype(nullptr)) const noexcept -> bool {
    return _data == nullptr;
  }
};

template <class T>
auto box(T val) -> Box<T> {
  return Box<T>::xnew(static_cast<T&&>(val));
}

}  // namespace sfc::boxed

namespace sfc {
using boxed::Box;
}  // namespace sfc
