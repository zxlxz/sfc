#pragma once

#include "sfc/alloc.h"

namespace sfc::backtrace {

class Frame {
  void* _addr;

 public:
  Frame(void* addr);
  ~Frame();

  auto func() const -> String;

  void fmt(auto& f) const {
    const auto s = this->func();
    f.write_str(s);
  }
};

}  // namespace sfc::backtrace
