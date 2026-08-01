#include "sfc/log/logger.h"

namespace sfc::log {

auto Logger::level() const -> Level {
  return _level;
}

void Logger::set_level(Level level) {
  _level = level;
}

void Logger::flush() {
  _backend.flush();
}

void Logger::write_str(Level level, Str message) {
  if (level < _level) {
    return;
  }

  const auto time = time::SystemTime::now();
  _backend.write(Record{time, level, fmt::Args{"{}", message}});
}

void Logger::write_fmt(Level level, fmt::Args args) {
  if (level < _level) {
    return;
  }

  const auto time = time::SystemTime::now();
  _backend.write(Record{time, level, args});
}

}  // namespace sfc::log
