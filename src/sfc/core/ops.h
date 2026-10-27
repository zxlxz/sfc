#pragma once

#include "sfc/core/num.h"

namespace sfc::ops {

template <class T>
auto declval() -> T;

template <class T, T... I>
struct IntSeq {
  static auto eq(const auto& a, const auto& b) -> bool {
    return ((a[I] == b[I]) && ...);
  }

  static auto ne(const auto& a, const auto& b) -> bool {
    return ((a[I] != b[I]) || ...);
  }

  static auto lt(const auto& a, const auto& b) -> bool {
    return ((a[I] < b[I]) && ...);
  }

  static auto le(const auto& a, const auto& b) -> bool {
    return ((a[I] <= b[I]) && ...);
  }

  static auto gt(const auto& a, const auto& b) -> bool {
    return ((a[I] > b[I]) && ...);
  }

  static auto ge(const auto& a, const auto& b) -> bool {
    return ((a[I] >= b[I]) && ...);
  }

  static auto all(const auto& a) -> bool {
    return (a[I] && ...);
  }

  static auto any(const auto& a) -> bool {
    return (a[I] || ...);
  }

  static auto accum(const auto& a) {
    return (a[I] + ...);
  }

  static auto prod(const auto& a) {
    return (a[I] * ...);
  }

  static auto dot(const auto& a, const auto& b) {
    return ((a[I] * b[I]) + ...);
  }
};

template <u32... I>
using IdxSeq = IntSeq<u32, I...>;

#if __has_builtin(__make_integer_seq)
template <u32 N>
using make_index_seq = __make_integer_seq<IntSeq, u32, N>;
#else
template <u32 N>
using make_index_seq = IntSeq<u32, __integer_pack(N)...>;
#endif

template <class>
struct Fn;

template <class>
struct FnMut;

template <class R, class... T>
struct Fn<R(T...)> {
  using mfn_t = R (*)(const void*, T&&...);
  using pfn_t = R (*)(T...);

  const void* _self;
  union {
    pfn_t _pfn;
    mfn_t _mfn;
  };

  template <class X>
  static constexpr auto kOpCall = [](const void* p, T&&... t) -> R { return (*(const X*)p)((T&&)t...); };

 public:
  explicit Fn(pfn_t pf) : _self{nullptr}, _pfn{pf} {}

  template <class X>
  explicit Fn(const X& x, mfn_t mf = Fn::kOpCall<X>) : _self{&x}, _mfn{mf} {}

 public:
  auto operator()(T... t) const -> R {
    return _self ? _mfn(_self, (T&&)t...) : _pfn((T&&)t...);
  }
};

template <class R, class... T>
struct FnMut<R(T...)> {
  using mfn_t = R (*)(void*, T&&...);
  using pfn_t = R (*)(T...);

  const void* _self;
  union {
    pfn_t _pfn;
    mfn_t _mfn;
  };

  template <class X>
  static constexpr auto kOpCall = [](void* p, T&&... t) -> R { return (*(X*)p)((T&&)t...); };

 public:
  explicit FnMut(pfn_t pf) : _self{nullptr}, _pfn{pf} {}

  template <class X>
  explicit FnMut(X& x, mfn_t mf = FnMut::kOpCall<X>) : _self{&x}, _mfn{mf} {}

 public:
  auto operator()(T... t) const -> R {
    return _self ? _mfn(_self, (T&&)t...) : _pfn((T&&)t...);
  }
};

template <class F, class... A>
using FnOut = decltype(declval<F>()(declval<A>()...));

}  // namespace sfc::ops

namespace sfc {
using ops::FnOut;
}  // namespace sfc
