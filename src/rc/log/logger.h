#pragma once

#include "rc/alloc.h"
#include "rc/time.h"
#include "rc/log/mod.h"

namespace rc::log {

using boxed::FnBox;
using time::Instant;

struct Logger {
  Level _level;
  Vec<FnBox<void(Entry)>> _backends;

  pub auto buff() -> Vec<u8>&;
  pub auto push(Level level, Slice<const u8> msg) -> void;

  template <class... Args>
  auto write(Level level, const Args&... args) -> void {
    if (level < _level) {
      return;
    }
    auto& buf = this->buff();
    fmt::write(buf, args...);
    this->push(level, buf.as_slice());
  }
};

pub auto logger() -> Logger&;

template <class... Args>
auto write(Level level, Str sfmt, const Args&... args) -> void {
  logger().write(level, sfmt, args...);
}

}  // namespace rc::log
