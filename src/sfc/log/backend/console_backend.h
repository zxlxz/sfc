#pragma once

#include "sfc/log/mod.h"

namespace sfc::log {

class ConsoleBackend : public IBackend {
 public:
  ConsoleBackend();
  ConsoleBackend(ConsoleBackend&&) noexcept;
  ~ConsoleBackend();

  void write_msg(Level level, Str msg) override;
};

}  // namespace sfc::log
