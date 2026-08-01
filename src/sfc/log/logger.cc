#include "sfc/log/logger.h"

namespace sfc::log {

static auto level_str(Level level) -> Str {
  switch (level) {
    case Level::Trace: return "[**]";
    case Level::Debug: return "[DD]";
    case Level::Info:  return "[II]";
    case Level::Warn:  return "[WW]";
    case Level::Error: return "[EE]";
    case Level::Fatal: return "[FF]";
  }
  return "[??]";
}

static auto time_str(time::SystemTime time) -> Str {
  static thread_local char buf[32];
  static thread_local auto out = Slice{buf};

  // write seconds, only when the seconds changed
  static thread_local auto prev_sec = u64{0};
  if (time.as_secs() != prev_sec) {
    prev_sec = time.as_secs();
    const auto t = time::DateTime::from_local(time);
    fmt::write(out, "{04}-{02}-{02} {02}:{02}:{02}.000", t.year, t.month, t.day, t.hour, t.minute, t.second);
  }

  {
    out._ptr -= 3;
    out._len += 3;
    const auto millis = time.subsec_millis();
    fmt::write(out, "{03}", millis);
  }

  auto str = Str{buf, sizeof(buf) - out.len()};
  return str;
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

  const auto time_str = log::time_str(time);
  const auto level_str = log::level_str(level);
  _backend.write(time_str, level_str, fmt::Args{"{}", message});
}

void Logger::write_fmt(Level level, fmt::Args args) {
  if (level < _level) {
    return;
  }

  const auto time = time::SystemTime::now();

  const auto time_str = log::time_str(time);
  const auto level_str = log::level_str(level);
  _backend.write(time_str, level_str, args);
}

}  // namespace sfc::log
