#include "sfc/log/logger.h"

namespace sfc::log {

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

}  // namespace sfc::log
