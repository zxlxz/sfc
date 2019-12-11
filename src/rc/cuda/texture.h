#pragma once

#include "rc/cuda/mod.h"

namespace rc::cuda {

template <class T, usize N>
struct Texture {
  static constexpr usize RANK = N;
  using Dims = math::NDDims<RANK>;
  using Desc = cuda::TexDesc;

  static_assert(rc::is_num<T>());
  static_assert(sizeof(T) <= sizeof(u32));
  static_assert(RANK >= 1 && RANK <= 3);

  TexImpl _imp;

  explicit Texture(TexImpl&& imp) : _imp{rc::move(_imp)} {}
  ~Texture() = default;
  Texture(Texture&&) noexcept = default;

  static auto with_dims_point(Dims dims, TexAddr addr) -> Texture {
    const auto xfmt = IntoArrXFmt<T>::into();
    const auto desc = TexDesc{addr, TexFilter::Point};
    const auto impl = TexImpl::with_dims(xfmt, dims, desc);
    return Texture{impl};
  }

  static auto with_dims_liner(Dims dims, TexAddr addr) -> Texture {
    const auto xfmt = IntoArrXFmt<T>::into();
    const auto desc = TexDesc{addr, TexFilter::Liner};
    const auto impl = TexImpl::with_dims(xfmt, dims, desc);
    return Texture { impl }
  }
};

}  // namespace rc::cuda
