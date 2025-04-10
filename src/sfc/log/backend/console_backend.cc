#include "console_backend.h"

#include "sfc/io.h"

namespace sfc::log {

static const Str COLOR_OFF = "\033[0m";

static inline auto level_str(Level level) -> Str {
  switch (level) {
    case Level::Trace:   return " [TT] ";
    case Level::Debug:   return " [DD] ";
    case Level::Info:    return " [II] ";
    case Level::Warning: return " [WW] ";
    case Level::Error:   return " [EE] ";
    case Level::Fatal:   return " [XX] ";
    default:             return " [??] ";
  }
}

static inline auto color_str(Level level) -> Str {
  switch (level) {
    case Level::Trace:   return "\033[39m";
    case Level::Debug:   return "\033[32m";
    case Level::Info:    return "\033[34m";
    case Level::Warning: return "\033[33m";
    case Level::Error:   return "\033[31m";
    case Level::Fatal:   return "\033[41m";
    default:             return "\033[34m";
  }
}

static inline auto fmt_logline(Entry entry, bool color) -> Str {
  static thread_local auto buf = String{};
  buf.clear();

  color ? buf.write_str(color_str(entry.level)) : void();
  buf.write_str(entry.time);
  buf.write_str(level_str(entry.level));
  color ? buf.write_str(COLOR_OFF) : void();
  buf.write_str(entry.msg);
  buf.write_str("\n");
  return buf.as_str();
}

ConsoleBackend::ConsoleBackend() {
  _enable_color = io::Stdout::is_tty();
}

ConsoleBackend::~ConsoleBackend() {}

void ConsoleBackend::set_color(bool value) {
  _enable_color = value;
}

void ConsoleBackend::flush() {
  io::Stdout::flush();
}

void ConsoleBackend::write_entry(Entry entry) {
  const auto line_str = fmt_logline(entry, _enable_color);

  io::Stdout::write_str(line_str);
}

}  // namespace sfc::log
