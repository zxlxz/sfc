#pragma once

#include "sfc/log/backend.h"

namespace sfc::log {

class ConsoleBackend {
  static constexpr usize BUFF_SIZE = 2048U;
  bool _enable_color = false;

 public:
  ConsoleBackend();
  ConsoleBackend(ConsoleBackend&&) noexcept;
  ~ConsoleBackend();

  void set_color(bool value);

  void flush();

  void write_entry(Entry entry);
};

}  // namespace sfc::log
