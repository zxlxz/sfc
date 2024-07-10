#pragma once

#include "sfc/log/backend.h"

namespace sfc::log {

class ConsoleBackend {
  bool _enable_color = false;

 public:
  ConsoleBackend();
  ConsoleBackend(ConsoleBackend&&) noexcept;
  ~ConsoleBackend();

  void set_color(bool value);

 public:
  void write_entry(Entry entry);
  auto make_log_str(Entry entry) const -> Str;
};

}  // namespace sfc::log
