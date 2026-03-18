#pragma once

#include "sfc/core.h"
#include "sfc/time.h"

namespace sfc::log {

enum class Level {
  Trace,
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
};

struct Record {
  time::SystemTime time;
  Level level;
  Str message;
};

struct IBackend {
  void flush();
  void push(const Record& entry);
};

}  // namespace sfc::log
