#include "logger.h"

#include "backend/console_backend.h"
#include "backend/file_backend.h"
#include "sfc/time/datetime.h"

namespace sfc::log {

struct LogTime {
  time::DateTime _inn;

 public:
  static auto now() -> LogTime {
    static auto cache_sys_time = time::System{};
    static auto cache_datetime = time::DateTime{};

    const auto sys_time = time::System::now();
    if (sys_time._secs != cache_sys_time._secs) {
      const auto date_time = time::DateTime::from(sys_time);
      cache_datetime = date_time;
      return LogTime{date_time};
    }

    return LogTime{cache_datetime};
  }

  auto to_str() const -> Str {
    static constexpr u32 DATE_LEN = sizeof("YYYY-MM-DD");
    static constexpr u32 TIME_LEN = sizeof("HH:MM:SS");
    static constexpr u32 LEN = sizeof("[YYYY-MM-DD HH:MM:SS.000]") - 1;

    static thread_local char buf[32] = "[YYYY-MM-DD HH:MM:SS.000]";
    static thread_local auto cached = time::DateTime{};

    if (!(_inn._date == cached._date)) {
      cached._date = _inn._date;

      const auto p = buf + 1;
      const auto y = _inn._date.year();
      const auto m = _inn._date.month();
      const auto d = _inn._date.day();
      __builtin_snprintf(p, DATE_LEN, "%04d-%02d-%02d", y, m, d);
      p[DATE_LEN - 1] = ' ';
    }

    if (!(_inn._time._secs == cached._time._secs)) {
      cached._time._secs = _inn._time._secs;

      const auto p = buf + DATE_LEN + 1;
      const auto h = _inn._time.hour();
      const auto m = _inn._time.minute();
      const auto s = _inn._time.second();
      __builtin_snprintf(p, TIME_LEN, "%02d:%02d:%02d", h, m, s);
      p[TIME_LEN - 1] = '.';
    }

    if (!(_inn._time.millis() == cached._time.millis())) {
      cached._time._micros = _inn._time._micros;

      const auto p = buf + DATE_LEN + TIME_LEN + 1;
      const auto m = _inn._time.millis();
      __builtin_snprintf(p, 4, "%03d", m);
      p[3] = ']';
    }

    return Str{buf, LEN};
  }
};

Logger::Logger() = default;

Logger::Logger(Logger&&) noexcept = default;

Logger::~Logger() {}

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

  const auto stime = LogTime::now().to_str();

  const auto entry = Entry{
      .level = level,
      .time = stime,
      .msg = msg,
  };

  for (auto& be : _backends.as_mut_slice()) {
    be->write_entry(entry);
  }
}

void Logger::add_backend_imp(Box<IBackend&> backend) {
  _backends.push(mem::move(backend));
}

void Logger::clear_backends() {
  _backends.clear();
}

}  // namespace sfc::log
