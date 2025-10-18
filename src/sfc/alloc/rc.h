#pragma once

#include "sfc/alloc/alloc.h"
#include "sfc/sync/atomic.h"

namespace sfc::rc {

using Layout = alloc::Layout;

template <class T, class A = alloc::Global>
class [[nodiscard]] Rc {
  struct Data {
    sync::Atomic<int> _cnt;
    T _val;
  };

  Data* _ptr = nullptr;
  [[no_unique_address]] A _alloc{};

 public:
  Rc() noexcept = default;

  ~Rc() noexcept {
    this->reset();
  }

  Rc(const Rc&) = delete;

  Rc& operator=(const Rc&) = delete;

  Rc(Rc&& other) noexcept : _ptr{other._ptr}, _alloc{static_cast<A&&>(other._alloc)} {
    other._ptr = nullptr;
  }

  auto operator=(Rc&& other) noexcept -> Rc& {
    if (this == &other) {
      return *this;
    }
    this->reset();
    _ptr = other._ptr, other._ptr = nullptr;
    _alloc = static_cast<A&&>(other._alloc);
    return *this;
  }

  static auto from_raw(Data* ptr, A alloc = {}) -> Rc {
    auto res = Rc{};
    res._ptr = ptr;
    res._alloc = static_cast<A&&>(alloc);
    return res;
  }

  static auto xnew(auto&&... args) -> Rc {
    auto res = Rc{};
    res._ptr = static_cast<Data*>(res._alloc.alloc(Layout::of<Data>()));
    try {
      new (res._ptr) Data{._cnt{1}, ._val{static_cast<decltype(args)&&>(args)...}};
    } catch (...) {
      res._alloc.dealloc(res._ptr, Layout::of<Data>());
      res._ptr = nullptr;
      throw;
    }
    return res;
  }

  auto clone() const -> Rc {
    if (!_ptr) {
      return {};
    }
    _ptr->_cnt.fetch_add(1);
    return Rc::from_raw(_ptr, _alloc);
  }

  explicit operator bool() const {
    return _ptr != nullptr;
  }

  auto operator->() const -> const T* {
    panicking::expect(_ptr != nullptr, "Rc::operator->: deref null");
    return &_ptr->_val;
  }

  auto operator->() -> T* {
    panicking::expect(_ptr != nullptr, "Rc::operator->: deref null");
    return &_ptr->_val;
  }

  auto operator*() const -> const T& {
    panicking::expect(_ptr != nullptr, "Rc::operator*: deref null");
    return _ptr->_val;
  }

  auto operator*() -> T& {
    panicking::expect(_ptr != nullptr, "Rc::operator*: deref null");
    return _ptr->_val;
  }

  void reset() noexcept {
    if (!_ptr) {
      return;
    }

    if (_ptr->_cnt.fetch_sub(1) == 1) {
      _ptr->_val.~T();
      _alloc.dealloc(_ptr, Layout::of<Data>());
    }
    _ptr = nullptr;
  }
};

}  // namespace sfc::rc

namespace sfc {
using rc::Rc;
}  // namespace sfc
