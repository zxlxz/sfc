#include "sfc/log/logger.h"

namespace sfc::log {

auto Sink::push(const Record& record) -> void {
  return (_vtbl->push)(_self, record);
}

void Sink::flush() {
  return (_vtbl->flush)(_self);
}

Logger::Logger(Sink sink) noexcept : _sink{sink} {}

Logger::~Logger() {}

Logger::Logger(Logger&&) noexcept = default;

Logger& Logger::operator=(Logger&&) noexcept = default;

auto Logger::level() const -> Level {
  return _level;
}

void Logger::set_level(Level level) {
  _level = level;
}

void Logger::flush() {
  _sink.flush();
}

void Logger::write_str(Level level, Str message) {
  if (level < _level) {
    return;
  }

  const auto time = time::SystemTime::now();
  _sink.push(Record{time, level, message});
}

void Logger::write_fmt(Level level, fmt::Args args) {
  if (level < _level) {
    return;
  }

  char raw_buf[1024];
  auto buf = fmt::Buf{raw_buf};

  const auto time = time::SystemTime::now();
  fmt::Formatter{buf}.write_val(args);
  _sink.push(Record{time, level, buf.as_str()});
}

}  // namespace sfc::log
