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
  _backend.push(Record{time, level, message});
}

void Logger::write_fmt(Level level, fmt::Args args) {
  if (level < _level) {
    return;
  }

  const auto time = time::SystemTime::now();

  auto buf = fmt::Buf<4096>{};
  fmt::Formatter{buf}.write_val(args);
  _backend.push(Record{time, level, buf.as_str()});
}

}  // namespace sfc::log
