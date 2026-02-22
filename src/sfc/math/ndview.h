#pragma once

#include "sfc/alloc.h"

namespace sfc::math {

struct Span {
  u32 idx;
  u32 end;

 public:
  Span(const u32 (&s)[1]) : idx{s[0]}, end{s[0]} {}
  Span(const u32 (&s)[2]) : idx{s[0]}, end{s[1]} {}

  auto operator%(u32 max_len) const -> Span {
    const auto idx = cmp::min(this->idx, max_len);
    const auto end = cmp::min(this->end, max_len);
    return Span{{idx, cmp::max(idx, end)}};
  }

  auto len() const -> u32 {
    return end - idx;
  }
};

template <class T, u32 N>
struct NdView;

template <u32... S>
struct NdSpan {
  static constexpr u32 N = sizeof...(S);
  static constexpr u32 M = (u32)(... + S) - N;
  Span _[sizeof...(S)];

  template <class T>
  auto operator()(NdView<T, N> view) const -> NdView<T, M> {
    static constexpr u32 K[] = {S...};
    auto res = NdView<T, M>{view._data};

    for (auto i = 0U, j = 0U; i < N; ++i) {
      const auto span = _[i] % view._shape[i];
      res._data += span.idx * view._strides[i];
      if (K[i] == 2) {
        res._shape[j] = span.len();
        res._strides[j] = view._strides[i];
        ++j;
      }
    }
    return res;
  }
};

template <u32... S>
NdSpan(const u32 (&... s)[S]) -> NdSpan<S...>;

template <class T, u32 N>
struct NdView;

template <class T>
struct NdView<T, 1> {
  static constexpr auto NDIM = 1;

  T* _data;
  u32 _shape[NDIM];
  u32 _strides[NDIM];

 public:
  auto numel() const -> u32 {
    return _shape[0];
  }

  auto operator[](u32 x) const -> T {
    return _data[x * _strides[0]];
  }

  auto operator[](u32 x) -> T& {
    return _data[x * _strides[0]];
  }

  auto operator[](const u32 (&x)[2]) -> NdView {
    return NdSpan{x}(*this);
  }
};

template <class T>
struct NdView<T, 2> {
  static constexpr auto NDIM = 2;

  T* _data;
  u32 _shape[NDIM];
  u32 _strides[NDIM];

 public:
  auto numel() const -> u32 {
    return _shape[0] * _shape[1];
  }

  auto operator[](u32 x, u32 y) const -> T {
    return _data[x * _strides[0] + y * _strides[1]];
  }

  auto operator[](u32 x, u32 y) -> T& {
    return _data[x * _strides[0] + y * _strides[1]];
  }

  template <u32 X, u32 Y>
  auto operator[](const u32 (&x)[X], const u32 (&y)[Y]) {
    return NdSpan{x, y}(*this);
  }
};

template <class T>
struct NdView<T, 3> {
  static constexpr auto NDIM = 3;
  T* _data;
  u32 _shape[NDIM];
  u32 _strides[NDIM];

 public:
  auto numel() const -> u32 {
    return _shape[0] * _shape[1] * _shape[2];
  }

  auto operator[](u32 x, u32 y, u32 z) const -> T {
    return _data[x * _strides[0] + y * _strides[1] + z * _strides[2]];
  }

  auto operator[](u32 x, u32 y, u32 z) -> T& {
    return _data[x * _strides[0] + y * _strides[1] + z * _strides[2]];
  }

  template <u32 X, u32 Y, u32 Z>
  auto operator[](const u32 (&x)[X], const u32 (&y)[Y], const u32 (&z)[Z]) {
    return NdSpan{x, y, z}(*this);
  }
};

template <class T>
struct NdView<T, 4> {
  static constexpr auto NDIM = 4;
  T* _data;
  u32 _shape[NDIM];
  u32 _strides[NDIM];

 public:
  auto numel() const -> u32 {
    return _shape[0] * _shape[1] * _shape[2] * _shape[3];
  }

  auto operator[](u32 x, u32 y, u32 z, u32 w) const -> T {
    return _data[x * _strides[0] + y * _strides[1] + z * _strides[2] + w * _strides[3]];
  }

  auto operator[](u32 x, u32 y, u32 z, u32 w) -> T& {
    return _data[x * _strides[0] + y * _strides[1] + z * _strides[2] + w * _strides[3]];
  }

  template <u32 X, u32 Y, u32 Z, u32 W>
  auto operator[](const u32 (&x)[X], const u32 (&y)[Y], const u32 (&z)[Z], const u32 (&w)[W]) {
    return NdSpan{x, y, z, w}(*this);
  }
};

}  // namespace sfc::math
