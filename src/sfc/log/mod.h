#pragma once

#include "sfc/collections.h"

namespace sfc::log {

enum class Level {
  Trace,
  Debug,
  Info,
  Warning,
  Error,
  Fatal,
};
SFC_ENUM(Level, Trace, Debug, Info, Warning, Error, Fatal)

class IBackend {
  friend class Box<IBackend>;

 protected:
  IBackend() = default;
  IBackend(IBackend&&) = default;
  virtual ~IBackend() = default;

 public:
  virtual void write_msg(Level level, Str msg) = 0;
};

}
