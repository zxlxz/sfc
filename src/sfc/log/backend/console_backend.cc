#include "console_backend.h"

#include "sfc/io.h"

namespace sfc::log {

static const Str COLOR_OFF = "\033[0m";

static inline auto level_str(Level level) -> Str {
  switch (level) {
    case Level::Trace:
      return " [--] ";
    case Level::Debug:
      return " [DD] ";
    case Level::Info:
      return " [II] ";
    case Level::Warning:
      return " [WW] ";
    case Level::Error:
      return " [EE] ";
    case Level::Fatal:
      return " [!!] ";
    default:
      return " [??] ";
  }
}

static inline auto color_str(Level level) -> Str {
  switch (level) {
    case Level::Trace:
      return "\033[39m";
    case Level::Debug:
      return "\033[32m";
    case Level::Info:
      return "\033[34m";
    case Level::Warning:
      return "\033[33m";
    case Level::Error:
      return "\033[31m";
    case Level::Fatal:
      return "\033[41m";
    default:
      return "\033[34m";
  }
}

ConsoleBackend::ConsoleBackend() {
  _enable_color = io::Stdout::is_tty();
}

ConsoleBackend::~ConsoleBackend() = default;

void ConsoleBackend::set_color(bool value) {
  _enable_color = value;
}

void ConsoleBackend::flush() {}

void ConsoleBackend::write_entry(Entry entry) {
  auto out = io::Stdout::lock();

  _enable_color ? out.write_str(color_str(entry.level)) : void();
  out.write_str(entry.time);
  out.write_str(level_str(entry.level));
  _enable_color ? out.write_str(COLOR_OFF) : void();
  out.write_str(entry.msg);
  out.write_str("\n");
}

}  // namespace sfc::log
