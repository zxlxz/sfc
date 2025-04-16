#include "logger.h"

#include "backend/console_backend.h"
#include "backend/file_backend.h"
#include "sfc/time/datetime.h"

namespace sfc::log {

Logger::Logger() = default;

Logger::~Logger() {}

Logger::Logger(Logger&&) noexcept = default;

Logger& Logger::operator=(Logger&&) noexcept = default;

auto Logger::get_level() const -> Level {
  return _level;
}

void Logger::set_level(Level level) {
  _level = level;
}

void Logger::flush() {
  for (auto& be : _backends.as_mut_slice()) {
    be->flush();
  }
}

void Logger::write_msg(Level level, Str msg) {
  if (level < _level) {
    return;
  }

  const auto datetime = time::DateTime::now();

  const auto entry = Entry{
      .level = level,
      .time = datetime.to_str(),
      .msg = msg,
  };

  for (auto& be : _backends.as_mut_slice()) {
    be->write_entry(entry);
  }
}

}  // namespace sfc::log
