#include "rc-test.inl"

#include "rc/cuda.h"

namespace rc::cuda {

rc_test(array) {
  cuda::set_device(Device::from_idx(0));

  auto h1 = math::NDArray<f32, 2>::with_dims({8, 8});
  h1 <<= math::Linspace{{1.0f, 0.1f}};

  auto d1 = cuda::NDArray<f32, 2>::with_dims(h1.dims());
  d1 <<= h1;

  auto h2 = math::NDArray<f32, 2>::with_dims(h1.dims());
  d1 >>= h2;

  log::info("h2 = {}", h2);
}

}  // namespace rc::cuda
