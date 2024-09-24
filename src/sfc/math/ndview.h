#pragma once

#include "sfc/alloc.h"

namespace sfc::math {

template <usize N>
struct NdSize {
  usize _inn[N] = {0};

  template <usize... I, class V>
  [[sfc_inline]] NdSize(tuple::idx_t<I...>, const V& v) : _inn{v[I]...} {}

 public:
  [[sfc_inline]] NdSize() = default;

  [[sfc_inline]] NdSize(const usize (&s)[N]) : NdSize{tuple::idx_seq_t<N>{}, s} {}

  [[sfc_inline]] auto operator[](usize idx) const -> usize {
    return _inn[idx];
  }

  template <usize... I>
  [[sfc_inline]] auto operator[](tuple::idx_t<I...>) const -> NdSize<sizeof...(I)> {
    return NdSize<sizeof...(I)>{_inn[I]...};
  }

  [[sfc_inline]] auto numel() const -> usize {
    const auto f = [&]<usize... I>(tuple::idx_t<I...>) { return (_inn[I] * ...); };
    return f(tuple::idx_seq_t<N>{});
  }

  [[sfc_inline]] auto operator==(const NdSize& other) const -> bool {
    const auto f = [&]<usize... I>(tuple::idx_t<I...>) { return ((_inn[I] == other._inn[I]) && ...); };
    return f(tuple::idx_seq_t<N>{});
  }

  void fmt(auto& f) const {
    f.write(_inn);
  }
};

template <usize N>
struct NdStep {
  usize _inn[N] = {0};

  template <usize... I, class V>
  [[sfc_inline]] NdStep(tuple::idx_t<I...>, const V& v) : _inn{v[I]...} {}

 public:
  [[sfc_inline]] NdStep() = default;

  [[sfc_inline]] NdStep(const usize (&s)[N]) : NdStep{tuple::idx_seq_t<N>{}, s} {}

  [[sfc_inline]] auto operator[](usize idx) const -> usize {
    return _inn[idx];
  }

  template <usize... I>
  [[sfc_inline]] auto operator[](tuple::idx_t<I...>) const -> NdStep<sizeof...(I)> {
    return NdStep<sizeof...(I)>{tuple::idx_t<I...>{}, _inn};
  }

  [[sfc_inline]] static auto from_shape(const NdSize<N>& shape) -> NdStep {
    auto res = NdStep{};

    res._inn[0] = 1;
    for (auto i = 1U; i < N; ++i) {
      res._inn[i] = res._inn[i - 1] * shape._inn[i - 1];
    }
    return res;
  }
};

template <usize N>
struct NdIdxs {
  usize _inn[N] = {0};

  template <usize... I, class V>
  [[sfc_inline]] NdIdxs(tuple::idx_t<I...>, const V& v) : _inn{v[I]...} {}

 public:
  [[sfc_inline]] NdIdxs() = default;

  [[sfc_inline]] NdIdxs(const usize (&s)[N]) : NdIdxs{tuple::idx_seq_t<N>{}, s} {}

  [[sfc_inline]] auto operator[](usize idx) const -> usize {
    return _inn[idx];
  }

  template <usize... I>
  [[sfc_inline]] auto operator[](tuple::idx_t<I...>) const -> NdIdxs<sizeof...(I)> {
    return NdIdxs<sizeof...(I)>{_inn[I]...};
  }

  [[sfc_inline]] auto operator<(const NdSize<N>& shape) const -> bool {
    const auto f = [&]<usize... I>(tuple::idx_t<I...>) { return ((_inn[I] < shape._inn[I]) && ...); };
    return f(tuple::idx_seq_t<N>{});
  }

  [[sfc_inline]] auto operator*(const NdStep<N>& strides) const -> usize {
    const auto f = [&]<usize... I>(tuple::idx_t<I...>) { return ((_inn[I] * strides._inn[I]) + ...); };
    return f(tuple::idx_seq_t<N>{});
  }
};

template <class T, usize N>
struct NdView {
  using shape_t = NdSize<N>;
  using strides_t = NdStep<N>;
  using Idxs = NdIdxs<N>;

  T* _data = nullptr;
  shape_t _size = {};
  strides_t _step = {};

 public:
  [[sfc_inline]] NdView() = default;

  [[sfc_inline]] NdView(T* data, shape_t shape, strides_t strides)
      : _data{data}, _size{shape}, _step{strides} {}

  [[sfc_inline]] static constexpr auto ndim() -> usize {
    return N;
  }

  [[sfc_inline]] auto data() const -> T* {
    return _data;
  }

  [[sfc_inline]] auto shape() const -> shape_t {
    return _size;
  }

  [[sfc_inline]] auto strides() const -> strides_t {
    return _step;
  }

  [[sfc_inline]] auto numel() const -> usize {
    return _size.numel();
  }

  [[sfc_inline]] auto operator[](usize idx) -> NdView<T, N - 1> {
    const auto data = _data + idx * _step[N - 1];
    return NdView<T, N - 1>{data, _size[tuple::idx_seq_t<N - 1>{}], _step[tuple::idx_seq_t<N - 1>{}]};
  }

  [[sfc_inline]] auto operator[](const Idxs& idx) const -> T {
    return _data[idx * _step];
  }

  [[sfc_inline]] auto operator[](const Idxs& idx) -> T& {
    return _data[idx * _step];
  }

  auto permute(Idxs idxs) -> NdView {
    usize size[N] = {};
    usize step[N] = {};
    for (auto i = 0U; i < N; ++i) {
      size[i] = _size[idxs[i]];
      step[i] = _step[idxs[i]];
    }
    return NdView{_data, size, step};
  }

  void fill(T val) {
    const auto n = _size[N - 1];
    for (auto i = 0U; i < n; ++i) {
      auto line = (*this)[i];
      line.fill(val);
    }
  }

  void copy_from(NdView src) {
    assert_fmt(_size == src._size, "NdView::copy_from: self.shape(={}) != src.shape(={})", _size,
               src._size);

    const auto n = _size[N - 1];
    for (auto i = 0U; i < n; ++i) {
      auto dst_line = (*this)[i];
      auto src_line = src[i];
      dst_line.copy_from(src_line);
    }
  }
};

template <class T>
struct NdView<T, 1> {
  using shape_t = NdSize<1>;
  using strides_t = NdStep<1>;
  using Idxs = NdIdxs<1>;

  T* _data = nullptr;
  shape_t _size = {};
  strides_t _step = {};

 public:
  [[sfc_inline]] NdView() = default;

  [[sfc_inline]] NdView(T* data, shape_t size, strides_t step) : _data{data}, _size{size}, _step{step} {}

  [[sfc_inline]] static constexpr auto ndim() -> usize {
    return 1;
  }

  [[sfc_inline]] auto data() const -> T* {
    return _data;
  }

  [[sfc_inline]] auto shape() const -> shape_t {
    return _size;
  }

  [[sfc_inline]] auto strides() const -> strides_t {
    return _step;
  }

  [[sfc_inline]] auto numel() const -> usize {
    return _size.numel();
  }

  [[sfc_inline]] auto operator[](usize idx) const -> T {
    return _data[idx * _step._inn[0]];
  }

  [[sfc_inline]] auto operator[](usize idx) -> T& {
    return _data[idx * _step._inn[0]];
  }

  [[sfc_inline]] auto operator[](const Idxs& ids) const -> T {
    return _data[ids._inn[0] * _step._inn[0]];
  }

  [[sfc_inline]] auto operator[](const Idxs& idxs) -> T& {
    return _data[idxs._inn[0] * _step._inn[0]];
  }

  auto operator[](Range<> ids) -> NdView {
    ids = ids % _size[0];

    const auto offset = ids._start * _step._inn[0];
    const auto size = shape_t{{ids.len()}};
    return NdView{_data + offset, size, _step};
  }

  void fill(T val) {
    const auto e = _data + _size[0] * _step[0];
    const auto k = _step[0];
    for (auto p = _data; p != e; p += k) {
      *p = val;
    }
  }

  void copy_from(const NdView& src) {
    assert_fmt(_size == src._size, "math::NdView::copy_from: .shape(={}) != src.shape(={})", _size,
               src._size);

    const auto pe = _data + _size[0] * _step[0];
    const auto ks = src._step[0];
    const auto kd = _step[0];
    for (auto ps = src._data, pd = _data; pd != pe; ps += ks, pd += kd) {
      *pd = *ps;
    }
  }
};

}  // namespace sfc::math
