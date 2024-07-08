#pragma once

#include "sfc/log/mod.h"

namespace sfc::log {

class ConsoleBackend {
 public:
  ConsoleBackend();
  ConsoleBackend(ConsoleBackend&&) noexcept;
  ~ConsoleBackend();

 public:
  void write_msg(Level level, Str msg);
};

}  // namespace sfc::log
