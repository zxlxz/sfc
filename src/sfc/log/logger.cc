#include "logger.h"

#include "backend/console_backend.h"
#include "backend/file_backend.h"
#include "sfc/time/datetime.h"

namespace sfc::log {

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

void Logger::write_msg(Level level, Str msg) {
  if (level < _level) {
    return;
  }

  const auto entry = Entry{
      .level = level,
      .time = this->make_time_str(),
      .msg = msg,
  };

  for (auto& be : _backends.as_mut_slice()) {
    be->write_entry(entry);
  }
}

auto Logger::get_tls_sbuf() -> String& {
  static thread_local auto res = String{};
  res.clear();
  return res;
}

auto Logger::make_time_str() -> Str {
  static constexpr u32 LEN = sizeof("YYYYMMDDTHH:MM:SS.UUUUUU") - 1;

  static thread_local char buf[32] = "YYYYMMDDTHH:MM:SS.UUUUUU";

  static auto old_date = time::NaiveDate{};
  static auto old_time = time::NaiveTime{};

  const auto date_time = time::DateTime::now_local();
  const auto date = date_time._date;
  const auto time = date_time._time;

  if (!(date == old_date)) {
    const auto p = buf;
    const auto y = date.year();
    const auto m = date.month();
    const auto d = date.day();

    __builtin_snprintf(p, 12, "%04d%02d%02d", y, m, d);
    p[sizeof("YYYYMMDD") - 1] = '-';
  }
  if (time._secs != old_time._secs) {
    const auto p = buf + sizeof("YYYYMMDD");
    const auto h = time.hour();
    const auto m = time.minute();
    const auto s = time.second();

    __builtin_snprintf(p, 10, "%02d:%02d:%02d", h, m, s);
    p[sizeof("HH:MM:SS") - 1] = '.';
  }

  if (true) {
    const auto p = buf + sizeof("YYYYMMDDTHH:MM:SS");
    const auto u = time.micros();
    __builtin_snprintf(p, 8, "%06d", u);
  }

  return Str{buf, LEN};
}

void Logger::add_backend(Box<IBackend&> backend) {
  _backends.push(mem::move(backend));
}

void Logger::clear_backends() {
  _backends.clear();
}

void Logger::add_console_backend() {
  auto backend = Box<ConsoleBackend>::xnew();
  this->add_backend(Box<IBackend&>(mem::move(backend)));
}

void Logger::add_file_backend(Str path) {
  auto backend = Box<FileBackend>::xnew(path);
  this->add_backend(Box<IBackend&>(mem::move(backend)));
}

}  // namespace sfc::log
