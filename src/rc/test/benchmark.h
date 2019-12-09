#pragma once

#include "rc/log.h"

namespace rc::test {

struct Benchmark {
  Str _name;
  usize _loop;

  Benchmark(Str name, usize loop = 1024) noexcept : _name(name), _loop(loop) {
    log::info(u8"{}", _name);
  }

  template <class F>
  auto operator()(Str name, F func) -> void {
    const auto t0 = time::Instant::now();
    for (usize i = 0; i < _loop; ++i) {
      func();
    }
    const auto t1 = time::Instant::now();
    const auto dt = t1.duration_since(t0);
    log::info(u8"  {}: {7.3}ms", name, dt.as_millis_f64());
  }
};

}  // namespace rc::test
