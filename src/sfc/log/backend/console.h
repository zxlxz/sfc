#pragma once

#include "sfc/log.h"

namespace sfc::log {

class ConsoleBackend {
  bool _color{false};

 public:
  explicit ConsoleBackend();
  ~ConsoleBackend();

  ConsoleBackend(ConsoleBackend&&) noexcept = default;
  ConsoleBackend& operator=(ConsoleBackend&&) noexcept = default;

  void set_color(bool value);
  void flush();
  void write(Record entry);
};

}  // namespace sfc::log
