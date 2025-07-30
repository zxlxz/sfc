#include "logger.h"

#include "backend/console_backend.h"
#include "backend/file_backend.h"
#include "sfc/chrono.h"

namespace sfc::log {

static auto make_time_str() -> str::Str {
  static thread_local char tls_buf[] = "0000-00-00 00:00:00.000";
  static thread_local auto tls_time = time::System{};

  auto& buf = tls_buf;

  auto fmt_uint = [](auto t, char* p, usize n) {
    for (; n != 0; n -= 1, t /= 10) {
      p[n - 1] = static_cast<char>('0' + t % 10);
    }
  };

  const auto now_time = time::System::now();
  const auto now_millis = now_time.sub_nanos() / time::NANOS_PER_MILLI;
  const auto tls_millis = tls_time.sub_nanos() / time::NANOS_PER_MILLI;
  if (now_millis != tls_millis) {
    fmt_uint(now_millis, buf + sizeof("0000-00-00 00:00:00"), 3);
  }

  const auto now_secs = now_time.secs();
  const auto tls_secs = tls_time.secs();
  if (now_secs / 60 != tls_secs / 60) {
    const auto date_time = chrono::DateTime::from_local(now_time);
    fmt_uint(date_time.year(), buf, 4);
    fmt_uint(date_time.month(), buf + sizeof("0000"), 2);
    fmt_uint(date_time.day(), buf + sizeof("0000-00"), 2);
    fmt_uint(date_time.hour(), buf + sizeof("0000-00-00"), 2);
    fmt_uint(date_time.minute(), buf + sizeof("0000-00-00 00"), 2);
  }
  if (now_secs != tls_secs) {
    fmt_uint(now_secs % 60, buf + sizeof("0000-00-00 00:00"), 2);
  }

  tls_time = now_time;
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
      .time = time_str,
      .msg = msg,
  };

  for (auto& be : _backends.as_mut_slice()) {
    be->write_entry(entry);
  }
}

}  // namespace sfc::log
