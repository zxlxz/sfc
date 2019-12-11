#include "rc-test.inl"

#include "rc/cuda.h"

namespace rc::cuda {

rc_test(device) {
  const auto dev_cnt = Device::count();
  log::info("dev_cnt = {}", dev_cnt);

  for (usize dev_idx = 0; dev_idx < dev_cnt; ++dev_idx) {
    const auto dev = Device::from_idx(dev_idx);
    log::info("dev: {}, arch={}", dev.name(), dev.arch());
    cuda::set_device(dev);
  }
}

}
