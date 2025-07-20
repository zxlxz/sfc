#pragma once

#include "sfc/io/stdio.h"
#include "sfc/log/backend.h"

namespace sfc::log {

class ConsoleBackend {
  io::Stdout _stdout{};
  bool       _color{false};

 public:
  explicit ConsoleBackend();

  ~ConsoleBackend();

  void set_color(bool value);

  void flush();

  void write_entry(Entry entry);
};

}  // namespace sfc::log
