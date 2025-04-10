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

struct Backtrace {
  Vec<void*> _frames;

 public:
  static auto capture() -> Backtrace;

  auto len() const -> usize {
    return _frames.len();
  }

  auto operator[](usize idx) const -> Frame {
    return Frame{_frames[idx]};
  }
};

}  // namespace sfc::backtrace
