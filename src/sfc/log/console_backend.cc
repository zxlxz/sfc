#include "sfc/log/console_backend.h"

#include "sfc/io.h"

namespace sfc::log {

static inline auto level_str(Level level) -> Str {
  switch (level) {
    case Level::Trace: return " [--] ";
    case Level::Debug: return " [DD] ";
    case Level::Info:  return " [II] ";
    case Level::Warn:  return " [WW] ";
    case Level::Error: return " [EE] ";
    case Level::Fatal: return " [!!] ";
    default:           return " [??] ";
  }
}

static inline auto color_str(Level level) -> Str {
  switch (level) {
    case Level::Trace: return "\033[39m";
    case Level::Debug: return "\033[32m";
    case Level::Info:  return "\033[34m";
    case Level::Warn:  return "\033[33m";
    case Level::Error: return "\033[31m";
    case Level::Fatal: return "\033[41m";
    default:           return "\033[39m";
  }
}

static inline auto color_off() -> Str {
  return "\033[0m";
}

ConsoleBackend::ConsoleBackend() : _color{io::Stdout::is_tty()} {}

ConsoleBackend::~ConsoleBackend() {}

void ConsoleBackend::set_color(bool value) {
  _color = value;
}

void ConsoleBackend::flush() {
  io::Stdout::flush();
}

void ConsoleBackend::write(Record entry) {
  // format buf
  static thread_local auto buf = String();
  const auto color_on = _color ? color_str(entry.level) : Str{};
  buf.clear();
  buf.push_str(color_on);
  buf.push_str(entry.time);
  buf.push_str(level_str(entry.level));
  buf.push_str(color_off());
  buf.push_str(entry.msg);
  buf.push_str("\n");

  // dump to stdout
  io::Stdout::write_str(buf.as_str());
  buf.clear();
}

}  // namespace sfc::log
