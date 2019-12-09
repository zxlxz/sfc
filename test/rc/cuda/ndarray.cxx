#include "rc-test.inl"

#include "rc/math.h"
#include "rc/cuda.h"
#include "rc/log.h"

namespace rc::cuda {

rc_test(linspace) {
  auto cpu_arr = math::NDArray<f32, 1>::with_dims({10});
  cpu_arr <<= math::Linspace({0.1f});
  auto gpu_arr = cuda::NDArray<f32, 1>::from_slice(cpu_arr);

  auto tmp_arr = math::NDArray<f32, 1>::with_dims({10});
  tmp_arr <<= gpu_arr;

  log::info(u8"tmp = {}", tmp_arr);
}

}  // namespace rc::cuda

