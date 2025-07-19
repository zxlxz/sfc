#include "logger.h"

#include "backend/console_backend.h"
#include "backend/file_backend.h"
#include "sfc/time/datetime.h"

namespace sfc::log {

static auto make_time_str() -> str::Str {
  static thread_local char tls_buf[] = "0000-00-00 00:00:00.000";
  static thread_local auto tls_time  = time::System{};

  auto fmt_uint = [](auto t, char* p, usize n) {
    for (; n != 0; n -= 1, t /= 10) {
      p[n - 1] = static_cast<char>('0' + t % 10);
    }
  };

  const auto p = tls_buf;
  auto&      t = tls_time;

  const auto current_time = time::System::now();
  if (current_time._micros != tls_time._micros) {
    const auto sub_micros = static_cast<u32>(current_time._micros % time::MICROS_PER_SEC);
    fmt_uint(sub_micros, p + sizeof("0000-00-00 00:00:00"), 3);
  }

  if (current_time.secs() != t.secs()) {
    const auto dt   = time::DateTime::from(current_time);
    const auto date = dt.date();
    const auto time = dt.time();

    fmt_uint(date.year(), p, 4);
    fmt_uint(date.month(), p + sizeof("0000"), 2);
    fmt_uint(date.day(), p + sizeof("0000-00"), 2);

    fmt_uint(time.hour(), p + sizeof("0000-00-00"), 2);
    fmt_uint(time.minute(), p + sizeof("0000-00-00 00"), 2);
    fmt_uint(time.second(), p + sizeof("0000-00-00 00:00"), 2);
  }

  tls_time = current_time;
  return str::Str{tls_buf};
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

  const auto time_str = make_time_str();

  const auto entry = Entry{
      .level = level,
      .time  = time_str,
      .msg   = msg,
  };

  for (auto& be : _backends.as_mut_slice()) {
    be->write_entry(entry);
  }
}

}  // namespace sfc::log
