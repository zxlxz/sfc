#pragma once

#include "sfc/core/mod.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

namespace sfc::ops {

template <class T>
auto declval() -> T;

template <class F, class... A>
using FnOut = decltype(declval<F>()(declval<A>()...));

template <class>
struct Dyn;

template <class R, class... T>
struct Dyn<R(T...)> {
  using pfn_t = R(T...);
  using mfn_t = R(void*, T...);

  const void* _obj;
  union {
    pfn_t* _pfun;
    mfn_t* _mfun;
  };

 public:
  Dyn() : _obj{nullptr}, _pfun{nullptr} {}

  Dyn(R (*func)(T...)) : _obj{nullptr}, _pfun{func} {}

  template <class X>
  Dyn(X& x) : _obj{&x}, _mfun{[](void* x, T... t) { return (*((X*)x))((T&&)t...); }} {}

  template <class X>
  Dyn(const X& x) : _obj{&x}, _mfun{[](void* x, T... t) { return (*((const X*)x))((T&&)t...); }} {}

  template <class X>
  Dyn(const X& x, R (*func)(const X&, T...)) : _obj{&x}, _mfun{(mfn_t*)func} {}

 public:
  R operator()(T... t) const {
    return _obj ? _mfun((void*)_obj, (T&&)t...) : _pfun((T&&)t...);
  }
};

struct End {};
static constexpr auto $ = End{};

struct Range {
  usize start;
  usize end;

 public:
  constexpr Range(usize start, usize end) : start{start}, end{end} {}
  constexpr Range(usize start, End) : start{start}, end{usize(-1)} {}

  constexpr auto wrap(usize len) const noexcept -> Range {
    const auto s = start < len ? start : len;
    const auto e = end < len ? end : len;
    return {s, e};
  }

  constexpr auto len() const noexcept -> usize {
    return start < end ? end - start : 0UL;
  }
};

}  // namespace sfc::ops

namespace sfc {
using ops::$;
using ops::Range;
using ops::FnOut;
}  // namespace sfc
