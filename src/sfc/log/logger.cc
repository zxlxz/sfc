#include "sfc/log/logger.h"

namespace sfc::log {

auto Record::level_str() const -> Str {
  switch (this->level) {
    case Level::Trace: return "--";
    case Level::Debug: return "DD";
    case Level::Info:  return "II";
    case Level::Warn:  return "WW";
    case Level::Error: return "EE";
    case Level::Fatal: return "!!";
    default:           return "??";
  }
}

auto Record::time_str() const -> Str {
  static constexpr auto kStrLen = sizeof("0000-00-00 00:00:00.000") - 1;
  static thread_local auto prev_sec = u64{0};

  static thread_local char buf[32];
  static thread_local auto out = fmt::SBuf{buf};

  const auto millis = time.subsec_millis();
  if (time.as_secs() != prev_sec) {
    prev_sec = time.as_secs();

    out = fmt::SBuf{buf};
    const auto t = time::DateTime::from_local(time);
    fmt::write(out, "{04}-{02}-{02} {02}:{02}:{02}.{03}", t.year, t.month, t.day, t.hour, t.minute, t.second, millis);
  } else {
    out._len = kStrLen - 3;  // trim off the old millis
    fmt::write(out, "{03}", millis);
  }
  return out.as_str();
}

void Record::fmt(fmt::Formatter& f) const {
  const auto time_str = this->time_str();
  const auto level_str = this->level_str();
  fmt::write(f, "{} [{}] {}", time_str, level_str, this->message);
}

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

  char buf[1024];
  auto out = fmt::SBuf{buf};
  fmt::Formatter{out}.write_val(args);

  const auto time = time::SystemTime::now();
  _backend.push(Record{time, level, out.as_str()});
}

}  // namespace sfc::log
