#pragma once

#include "sfc/log.h"

namespace sfc::log {

class ConsoleBackend : public IBackend {
  bool _color{false};

 public:
  explicit ConsoleBackend();
  ~ConsoleBackend();

  void set_color(bool value);
  void flush() override;
  void write(Record entry) override;
};

}  // namespace sfc::log
