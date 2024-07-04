#pragma once

#include "mod.h"

namespace sfc::math {

template <usize N>
struct NdSize {
  usize _inn[N] = {0};

  template <usize... I, class V>
  [[sfc_inline]] NdSize(idx_t<I...>, const V& v) : _inn{v[I]...} {}

 public:
  [[sfc_inline]] NdSize() = default;

  [[sfc_inline]] NdSize(const usize (&s)[N]) : NdSize{idx_seq_t<N>{}, s} {}

  [[sfc_inline]] auto operator[](usize idx) const -> usize {
    return _inn[idx];
  }

  template <usize... I>
  [[sfc_inline]] auto operator[](idx_t<I...>) const -> NdSize<sizeof...(I)> {
    return NdSize<sizeof...(I)>{_inn[I]...};
  }

  [[sfc_inline]] auto numel() const -> usize {
    const auto f = [&]<usize... I>(idx_t<I...>) { return (_inn[I] * ...); };
    return f(idx_seq_t<N>{});
  }

  [[sfc_inline]] auto operator==(const NdSize& other) const -> bool {
    const auto f = [&]<usize... I>(idx_t<I...>) { return ((_inn[I] == other._inn[I]) && ...); };
    return f(idx_seq_t<N>{});
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

  [[sfc_inline]] NdStep(const usize (&s)[N]) : NdStep{idx_seq_t<N>{}, s} {}

  [[sfc_inline]] auto operator[](usize idx) const -> usize {
    return _inn[idx];
  }

  template <usize... I>
  [[sfc_inline]] auto operator[](idx_t<I...>) const -> NdStep<sizeof...(I)> {
    return NdStep<sizeof...(I)>{idx_t<I...>{}, _inn};
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
  [[sfc_inline]] NdIdxs(idx_t<I...>, const V& v) : _inn{v[I]...} {}

 public:
  [[sfc_inline]] NdIdxs() = default;

  [[sfc_inline]] NdIdxs(const usize (&s)[N]) : NdIdxs{tuple::idx_seq_t<N>{}, s} {}

  [[sfc_inline]] auto operator[](usize idx) const -> usize {
    return _inn[idx];
  }

  template <usize... I>
  [[sfc_inline]] auto operator[](idx_t<I...>) const -> NdIdxs<sizeof...(I)> {
    return NdIdxs<sizeof...(I)>{_inn[I]...};
  }

  [[sfc_inline]] auto operator<(const NdSize<N>& shape) const -> bool {
    const auto f = [&]<usize... I>(idx_t<I...>) { return ((_inn[I] < shape._inn[I]) && ...); };
    return f(idx_seq_t<N>{});
  }

  [[sfc_inline]] auto operator*(const NdStep<N>& strides) const -> usize {
    const auto f = [&]<usize... I>(idx_t<I...>) { return ((_inn[I] * strides._inn[I]) + ...); };
    return f(idx_seq_t<N>{});
  }
};

template <class T, usize N>
struct NdView {
  using Size = NdSize<N>;
  using Step = NdStep<N>;
  using Idxs = NdIdxs<N>;

  T* _data = nullptr;
  Size _size = {};
  Step _step = {};

 public:
  [[sfc_inline]] NdView() = default;

  [[sfc_inline]] NdView(T* data, Size shape, Step strides)
      : _data{data}, _size{shape}, _step{strides} {}

  [[sfc_inline]] static constexpr auto ndim() -> usize {
    return N;
  }

  [[sfc_inline]] auto data() const -> T* {
    return _data;
  }

  [[sfc_inline]] auto shape() const -> Size {
    return _size;
  }

  [[sfc_inline]] auto strides() const -> Step {
    return _step;
  }

  [[sfc_inline]] auto numel() const -> usize {
    return _size.numel();
  }

  [[sfc_inline]] auto operator[](usize idx) -> NdView<T, N - 1> {
    const auto data = _data + idx * _step[N - 1];
    return NdView<T, N - 1>{data, _size[idx_seq_t<N - 1>{}], _step[idx_seq_t<N - 1>{}]};
  }

  [[sfc_inline]] auto operator[](const Idxs& idx) const -> T {
    return _data[idx * _step];
  }

  [[sfc_inline]] auto operator[](const Idxs& idx) -> T& {
    return _data[idx * _step];
  }

  auto transpose(Idxs idxs) -> NdView {
    usize size[N] = {};
    usize step[N] = {};
    for (auto i = 0U; i < N; ++i) {
      size[i] = _size[idxs[i]];
      step[i] = _step[idxs[i]];
    }
    return NdView{_data, size, step};
  }

  void fill(T val) {
    const auto n = this->len();
    for (auto i = 0U; i < n; ++i) {
      auto line = (*this)[i];
      line.fill(val);
    }
  }

  void copy_from(NdView src) {
    assert_fmt(_size == src._size, "NdView::copy_from: self.shape(={}) != src.shape(={})", _size,
               src._size);

    const auto n = this->len();
    for (auto i = 0U; i < n; ++i) {
      auto dst_line = (*this)[i];
      auto src_line = src[i];
      dst_line.copy_from(src_line);
    }
  }
};

template <class T>
struct NdView<T, 1> {
  using Size = NdSize<1>;
  using Step = NdStep<1>;
  using Idxs = NdIdxs<1>;

  T* _data = nullptr;
  Size _size = {};
  Step _step = {};

 public:
  [[sfc_inline]] NdView() = default;

  [[sfc_inline]] NdView(T* data, Size size, Step step) : _data{data}, _size{size}, _step{step} {}

  [[sfc_inline]] static constexpr auto ndim() -> usize {
    return 1;
  }

  [[sfc_inline]] auto data() const -> T* {
    return _data;
  }

  [[sfc_inline]] auto shape() const -> Size {
    return _size;
  }

  [[sfc_inline]] auto strides() const -> Step {
    return _step;
  }

  [[sfc_inline]] auto len() const -> usize {
    return _size[0];
  }

  [[sfc_inline]] auto numel() const -> usize {
    return _size.numel();
  }

  [[sfc_inline]] auto operator[](const Idxs& idxs) const -> T {
    return _data[idxs._inn[0] * _step._inn[0]];
  }

  [[sfc_inline]] auto operator[](const Idxs& idxs) -> T& {
    return _data[idxs._inn[0] * _step._inn[0]];
  }

  [[sfc_inline]] auto operator[](usize idx) const -> T {
    return _data[idx * _step._inn[0]];
  }

  [[sfc_inline]] auto operator[](usize idx) -> T& {
    return _data[idx * _step._inn[0]];
  }

  auto slice(NdIdxs<1> idxs, NdSize<1> size) -> NdView {
    const auto data = _data + idxs * _step;
    const auto shape = cmp::max(idxs[0] + size[0], _size[0]) - idxs[0];
    return NdView{data, {shape}, _step};
  }

  auto permute([[maybe_unused]] NdIdxs<1> idxs) -> NdView {
    return *this;
  }

  void fill(const T& val) {
    auto p = _data;
    auto e = p + _size[0] * _step[0];
    for (; p != e; p += _step[0]) {
      *p = val;
    }
  }

  void copy_from(const NdView& src) {
    assert_fmt(_size == src._size, "NdView::copy_from: self.shape(={}) != src.shape(={})", _size,
               src._size);

    auto y = _data;
    auto x = src._data;
    auto e = x + _size[0] * _step[0];
    for (; x != e; y += _step[0], x += _step[0]) {
      *y = *x;
    }
  }
};

}  // namespace sfc::math
