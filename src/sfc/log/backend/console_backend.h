#pragma once

#include "sfc/log/backend.h"

namespace sfc::log {

class ConsoleBackend {
  bool _enable_color = false;

 public:
  explicit ConsoleBackend();
  ~ConsoleBackend();

  void set_color(bool value);

  void flush();

  void write_entry(Entry entry);
};

}  // namespace sfc::log
