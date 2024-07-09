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

}  // namespace sfc::log
