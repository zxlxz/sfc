#pragma once

#include "sfc/alloc.h"

namespace sfc::backtrace {

struct Frame {
  void* _addr;

 public:
  auto to_str(Slice<char> buf) const -> Str;

  void fmt(auto& f) const {
    char buf[256];
    const auto s = this->to_str({buf});
    f.write_str(s);
  }
};

auto capture() -> Vec<Frame>;

}  // namespace sfc::backtrace
