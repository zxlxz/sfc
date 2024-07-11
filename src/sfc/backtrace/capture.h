#pragma once

#include "frame.h"

namespace sfc::backtrace {

class Backtrace {
  Vec<Frame> _frames;

  Backtrace(Vec<Frame> frames) noexcept : _frames{mem::move(frames)} {}

 public:
  Backtrace() noexcept = default;
  ~Backtrace() = default;

  Backtrace(Backtrace&&) noexcept = default;

  Backtrace& operator=(Backtrace&&) noexcept = default;

  static auto capture() noexcept -> Backtrace;

  auto frames() const -> Slice<const Frame> {
    return _frames.as_slice();
  }

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
