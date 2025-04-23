#include "logger.h"

#include "backend/console_backend.h"
#include "backend/file_backend.h"
#include "sfc/time/datetime.h"

namespace sfc::log {

static void fast_format_time(char (&buf)[sizeof("0000-00-00 00:00:00.000000")]) {
  static thread_local auto cached_time = time::System{};

  const auto current_time = time::System::now();

  if (current_time._micros != cached_time._micros) {
    const auto sub_micros = static_cast<u32>(current_time._micros % time::MICROS_PER_SEC);
    __builtin_snprintf(buf + sizeof("0000-00-00 00:00:00"), 7, "%06u", sub_micros);
  }

  if (current_time.secs() != cached_time.secs()) {
    const auto dt = time::DateTime::from(current_time);
    const auto date = dt.date();
    const auto time = dt.time();
    __builtin_snprintf(buf,
                       sizeof(buf),
                       "%04u-%02u-%02u %02u:%02u:%02u.%06u",
                       date.year(),
                       date.month(),
                       date.day(),
                       time.hour(),
                       time.minute(),
                       time.second(),
                       time.micros());
  }

  cached_time = current_time;
}

Logger::Logger() = default;

Logger::~Logger() = default;

Logger::Logger(Logger&&) noexcept = default;

auto Logger::operator=(Logger&&) noexcept -> Logger& = default;

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

  static thread_local char time_buf[] = "0000-00-00 00:00:00.000000";
  fast_format_time(time_buf);

  const auto entry = Entry{
      .level = level,
      .time = time_buf,
      .msg = msg,
  };

  for (auto& be : _backends.as_mut_slice()) {
    be->write_entry(entry);
  }
}

}  // namespace sfc::log
