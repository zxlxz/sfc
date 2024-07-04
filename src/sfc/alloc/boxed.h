#pragma once

#include "alloc.h"

namespace sfc::boxed {

template <class T, class A = alloc::Global>
class Box {
  ptr::Unique<T> _ptr{};
  A _a{};

  explicit Box(T* ptr) noexcept : _ptr{ptr} {}

  template <class... U>
  explicit Box(mem::inplace_t, U&&... args) {
    const auto p = _a.template alloc_one<T>();
    try {
      new (mem::inplace_t{}, p) T{static_cast<U&&>(args)...};
      _ptr = ptr::Unique<T>{p};
    } catch (...) {
      _a.dealloc_one(p);
      throw;
    }
  }

 public:
  Box() noexcept = default;

  ~Box() {
    if (!_ptr) {
      return;
    }
    _ptr->~T();
    _a.dealloc_one(_ptr._ptr);
  }

  Box(Box&& other) noexcept = default;

  Box& operator=(Box&& other) noexcept {
    auto tmp = static_cast<Box&&>(other);
    mem::swap(_ptr, tmp._ptr);
    return *this;
  }

  template <class... U>
  static auto xnew(U&&... args) -> Box {
    return Box{mem::inplace_t{}, static_cast<U&&>(args)...};
  }

  static auto from_raw(T* ptr) -> Box {
    return Box{ptr};
  }

  [[sfc_inline]] operator bool() const noexcept {
    return _ptr._ptr != nullptr;
  }

  [[sfc_inline]] auto ptr() const -> T* {
    return _ptr._ptr;
  }

  [[sfc_inline]] auto operator->() const -> const T* {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr._ptr;
  }

  [[sfc_inline]] auto operator->() -> T* {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::->: deref null");
    return _ptr._ptr;
  }

  [[sfc_inline]] auto operator*() const -> const T& {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr._ptr;
  }

  [[sfc_inline]] auto operator*() -> T& {
    assert_fmt(_ptr._ptr != nullptr, "boxed::Box::*: deref null");
    return *_ptr._ptr;
  }

  auto into_raw() && -> T* {
    const auto p = _ptr._ptr;
    _ptr._ptr = nullptr;
    return p;
  }

  template <class B>
  auto downcast() && -> Box<B> {
    static_assert(__has_virtual_destructor(B));

    auto ptr = static_cast<Box&&>(*this).into_raw();
    auto res = Box<B>::from_raw(ptr);
    return res;
  }
};

template <class R, class... T>
class Box<R(T...)> {
  struct Meta {
    void (*dtor)(void*);
    R (*call)(void*, T...);

    template <class X>
    static auto of() -> const Meta& {
      static const auto res = Meta{
          .dtor = [](void* p) { static_cast<X*>(p)->~X(); },
          .call = [](void* p, T... t) { return (*static_cast<X*>(p))(static_cast<T&&>(t)...); },
      };
      return res;
    }
  };

  class Data {
    friend class Box;
    friend class Box<Data>;

    const Meta* _meta;
    u64 _data[1];

    Data() = delete;

    Data(const Data&) = delete;

    ~Data() {
      _meta->dtor(_data);
    }

    auto invoke(auto&&... args) -> R {
      return (_meta->call)(_data, static_cast<decltype(args)&&>(args)...);
    }
  };

  template <class X>
  struct Impl {
    const Meta* _meta = &Meta::template of<X>();
    X _data;
    Impl(auto&&... args) : _data{static_cast<decltype(args)&&>(args)...} {}

    static auto xnew(auto&&... args) -> Box<Data> {
      auto box = Box<Impl>::xnew(args...);
      auto ptr = static_cast<Box<Impl>&&>(box).into_raw();
      auto res = Box<Data>::from_raw(mem::bit_cast<Data*>(ptr));
      return res;
    }
  };

  using Inn = Box<Data>;
  Inn _inn;

  Box(Inn inn) noexcept : _inn{static_cast<Inn&&>(inn)} {}

 public:
  Box() = default;

  template <class X>
  explicit Box(X x) noexcept : Box{Impl<X>::xnew(static_cast<X&&>(x))} {}

  ~Box() = default;

  Box(Box&& other) noexcept = default;

  Box& operator=(Box&& other) noexcept = default;

  static auto from_raw(void* raw) -> Box {
    return Box{Inn::from_raw(static_cast<Data*>(raw))};
  }

  auto into_raw() && -> void* {
    return static_cast<Inn&&>(_inn).into_raw();
  }

  explicit operator bool() const {
    return bool(_inn);
  }

  auto operator()(T... args) -> R {
    assert_fmt(_inn, "Box::(): deref null object.");
    return _inn->invoke(static_cast<T&&>(args)...);
  }
};

}  // namespace sfc::boxed

namespace sfc {
using boxed::Box;
}  // namespace sfc
