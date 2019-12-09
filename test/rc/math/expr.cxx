#include "rc-test.inl"
#include "rc/math.h"
#include "rc/log.h"

namespace rc::math {

rc_test(linspace) {
  auto n = 10u;
  auto a = NDArray<f32, 1>::with_dims({n});
  a <<= math::Linspace{{0.1f}};
  log::info(u8"a1 = {}", a);
}

rc_test(bop_1d) {
  auto n = 10u;

  auto a = NDArray<f32, 1>::with_dims({n});
  auto b = NDArray<f32, 1>::with_dims({n});
  auto c = NDArray<f32, 1>::with_dims({n});

  a <<= 1.0f;
  b <<= Linspace{{0.1f}};

  c <<= a + b;
  log::info(u8"a+b = {}", c);

  c <<= a - b;
  log::info(u8"a-b = {}", c);

  c <<= a * b;
  log::info(u8"a*b = {}", c);

  c <<= a / b;
  log::info(u8"a/b = {}", c);
}

rc_test(ndarray_2d_slice) {
  auto n = usize(10);
  auto a = math::NDArray<f32, 2>::with_dims({n, n});
  a <<= math::Linspace{{0.1f, 1.0f}};

  log::info("a = {4.2}", a);
  log::info("a[:,:] = {4.2}", a.slice(_, _));
  log::info("a[1,:] = {4.2}", a.slice({1ull}, _));
  log::info("a[:,1] = {4.2}", a.slice(_, {0ull, 1ull}));
  log::info("a[0:2, 0:2] = {4.2}", a.slice({0ull, 2ull}, {0ull, 2ull}));
}

}  // namespace rc::math
