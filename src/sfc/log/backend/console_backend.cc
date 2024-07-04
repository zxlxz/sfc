#include "console_backend.h"

#include "sfc/io.h"
#include "sfc/time.h"

namespace sfc::log {

namespace imp {
static auto make_time_str() -> Str {
  static thread_local char buf[32];

  static const auto time_start = time::Instant::now();
  const auto time_dur = time_start.elpased().as_secs_f64();
  const auto buf_len = __builtin_snprintf(buf, sizeof(buf), "[%10.6f] ", time_dur);
  if (buf_len <= 0) {
    return {};
  }
  return Str{buf, static_cast<usize>(buf_len)};
}

static auto make_level_color(Level level) -> Str {
  switch (level) {
    case Level::Trace:   return "\033[39m";
    case Level::Debug:   return "\033[32m";
    case Level::Info:    return "\033[34m";
    case Level::Warning: return "\033[33m";
    case Level::Error:   return "\033[31m";
    case Level::Fatal:   return "\033[41m";
  }
  return "";
}

static auto make_level_str(Level level) -> Str {
  switch (level) {
    case Level::Trace:   return "[TT]";
    case Level::Debug:   return "[DD]";
    case Level::Info:    return "[II]";
    case Level::Warning: return "[WW]";
    case Level::Error:   return "[EE]";
    case Level::Fatal:   return "[XX]";
  }
  return "[??]";
}

static auto make_log_str(Level level, Str msg) {
  const auto time_str = imp::make_time_str();
  const auto level_str = imp::make_level_str(level);
  const auto color_str = imp::make_level_color(level);
  const auto color_off = "\033[39;49m";

  static thread_local String buf;
  buf.clear();
  buf.push_str(color_str);
  buf.push_str(time_str);
  buf.push_str(level_str);
  buf.push_str(color_off);
  buf.push_str(" ");
  buf.push_str(msg);
  buf.push_str("\n");

  return buf.as_str();
}

}  // namespace imp

ConsoleBackend::ConsoleBackend() {}

ConsoleBackend::ConsoleBackend(ConsoleBackend&&) noexcept = default;

ConsoleBackend::~ConsoleBackend() {}

void ConsoleBackend::write_msg(Level level, Str msg) {
  static constexpr auto MAX_MSG_LEN = 4096u;
  msg = msg[{0, MAX_MSG_LEN}];

  const auto s = imp::make_log_str(level, msg);
  io::Stdout::instance().write_str(s);
}

}  // namespace sfc::log
