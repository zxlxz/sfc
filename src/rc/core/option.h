#pragma once

#include "rc/core/ops.h"
#include "rc/core/cmp.h"

namespace rc::option {

using ops::Invoke;

enum class Tag : u8 {
  None,
  Some,
};

template <class T>
struct Option {
  union {
    T _val;
    u8 _nil = 0;
  };
  Tag _tag;

  constexpr explicit Option() noexcept : _nil{0}, _tag{Tag::None} {}

  constexpr explicit Option(T val) noexcept
      : _val{rc::move(val)}, _tag{Tag::Some} {}

  auto is_none() const noexcept -> bool { return _tag == Tag::None; }

  auto is_some() const noexcept -> bool { return _tag == Tag::Some; }

  auto as_some() const noexcept -> const T& { return _val; }

  /* cmp: Eq<Option<T>>*/
  template <class U>
  auto eq(const Option<U>& other) const noexcept -> bool {
    if (_tag == Tag::None) return other.is_none();
    if (other.is_some()) {
      return _val == other._val;
    }
    return false;
  }

  /* unwarp: move */
  auto unwrap() && -> T {
    if (_tag == Tag::None) {
      rc::panic(u8"Option::unwrap(): None");
    }
    return rc::move(_val);
  }

  /* unwarp_or: move */
  auto unwrap_or(T default_val) && -> T {
    if (_tag == Tag::Some) {
      return rc::move(_val);
    }
    return rc::move(default_val);
  }

  /* op[+]: ref*/
  auto operator+(T other) && noexcept -> Option<T> {
    if (_tag == Tag::None) return {};
    return {_val + rc::move(other)};
  }

  /* ops: - */
  auto operator-(T other) const noexcept -> Option<T> {
    if (_tag == Tag::None) return {};
    return {_val - rc::move(other)};
  }
};

template <>
struct Option<void> {
  union {
    unit _val;
    u8 _nil = 0;
  };
  Tag _tag;
  constexpr explicit Option() noexcept : _tag{Tag::None} {}
  constexpr explicit Option(unit) noexcept : _tag{Tag::Some} {}

  auto is_none() const noexcept -> bool { return _tag == Tag::None; }
  auto is_some() const noexcept -> bool { return _tag == Tag::Some; }
  auto as_some() const noexcept -> unit { return {}; }
};

template <class T>
struct Option<T&> {
  T* _ptr;

  constexpr explicit Option() noexcept : _ptr{nullptr} {}
  constexpr explicit Option(T& ref) noexcept : _ptr{&ref} {}

  auto is_none() const noexcept -> bool { return _ptr == nullptr; }
  auto is_some() const noexcept -> bool { return _ptr != nullptr; }
  auto as_some() const noexcept -> const T& { return *_ptr; }
};

template <class T>
Option(T&&)->Option<T>;

template <class T>
Option(T&)->Option<T&>;

template <class T, class U>
inline auto operator&&(const Option<T>& self, Option<U> optb) -> U {
  if (self.is_none()) return {};
  return rc::move(optb);
}

template <class T>
inline auto operator||(Option<T> self, Option<T> optb) noexcept -> Option<T> {
  if (self.is_some()) return rc::move(self);
  return rc::move(optb);
}

template <class T, class F, class U = ops::invoke_t<F(const T&)>>
inline auto operator|(Option<T> self, F&& f) -> Option<U> {
  if (self.is_some() == Tag::None) return {};
  return {f(rc::move(self._val))};
}

}  // namespace rc::option

namespace rc {
using option::Option;
}
