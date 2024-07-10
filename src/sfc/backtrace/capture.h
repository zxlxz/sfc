#pragma once

#include "frame.h"

namespace sfc::backtrace {

class Backtrace {
  Vec<Frame> _frames;

 public:
  Backtrace() noexcept;
  Backtrace(Vec<Frame> frames) noexcept;
  Backtrace(Backtrace&&) noexcept;
  ~Backtrace();

  auto operator=(Backtrace&&) noexcept -> Backtrace&;

  static auto capture() noexcept -> Backtrace;

  auto frames() const -> Slice<const Frame>;

  void fmt(auto& f) const {
    auto fs = this->frames();

    auto i = 0U;
    for (auto& frame : fs) {
      f.write_fmt("[{>2d}] {}\n", i, frame);
      i += 1;
    }
  }
};

}  // namespace sfc::backtrace
