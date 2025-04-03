#pragma once

#include "sfc/alloc.h"

namespace sfc::backtrace {

struct Frame {
  void* _addr;

 public:
  auto func() const -> String;

  void fmt(auto& f) const {
    const auto s = this->func();
    f.write_str(s);
  }
};

auto capture() -> Vec<Frame>;

}  // namespace sfc::backtrace
