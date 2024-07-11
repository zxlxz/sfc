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
  auto make_plain_log(Entry entry) const -> Str;
  auto make_color_log(Entry entry) const -> Str;
};

}  // namespace sfc::log
