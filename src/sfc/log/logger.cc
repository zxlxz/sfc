#include "sfc/log/logger.h"

namespace sfc::log {

auto Record::time_str() const -> Str {
  static thread_local auto prev_sec = u64{0};
  static thread_local auto buf = fmt::FixedBuf<32>{};

  if (time.as_secs() != prev_sec) {
    prev_sec = time.as_secs();

    const auto t = time::DateTime::from_local(time);
    buf.clear();
    fmt::write(buf,
               "{04}-{02}-{02} {02}:{02}:{02}.",
               t.year,
               t.month,
               t.day,
               t.hour,
               t.minute,
               t.second);
  }
  fmt::write(buf, "{03}", time.subsec_millis());
  return buf.as_str();
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
