#pragma once

#include "sfc/alloc.h"

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
  Level level;
  Str time;
  Str msg;

 public:
  static auto tls_buf() -> String&;
  static auto time_str() -> Str;
};

class IBackend {
 public:
  IBackend() = default;
  virtual ~IBackend() = default;

  virtual void flush() = 0;
  virtual void write(Record entry) = 0;
};

}  // namespace sfc::log
