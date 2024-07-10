#include "console_backend.h"

#include "sfc/io.h"
#include "sfc/time.h"

namespace sfc::log {

ConsoleBackend::ConsoleBackend() {
  auto& stdout = io::Stdout::instance();
  _enable_color = stdout.is_tty();
}

ConsoleBackend::ConsoleBackend(ConsoleBackend&&) noexcept = default;

ConsoleBackend::~ConsoleBackend() {}

void ConsoleBackend::set_color(bool value) {
  _enable_color = value;
}

void ConsoleBackend::write_entry(Entry entry) {
  const auto s = this->make_log_str(entry);
  io::Stdout::instance().write_str(s);
}

auto ConsoleBackend::make_log_str(Entry entry) const -> Str {
  static const u64 LEVEL_CNT = static_cast<u64>(Level::Fatal) + 1;

  static const Str LEVEL_STR[] = {
      "[TT]", "[DD]", "[II]", "[WW]", "[EE]", "[XX]", "[??]",
  };

  static const Str COLOR_OFF = "\033[39;49m";
  static const Str COLOR_STR[] = {
      "\033[39m", "\033[32m", "\033[34m", "\033[33m", "\033[31m", "\033[41m", "",
  };

  const auto level_id = cmp::min(static_cast<u64>(entry.level), LEVEL_CNT);
  const auto level_ss = LEVEL_STR[level_id];
  const auto color_ss = COLOR_STR[level_id];

  static thread_local String buf;
  buf.clear();

  if (_enable_color) {
    buf.push_str(color_ss);
    buf.push_str(entry.time);
    buf.push_str(COLOR_OFF);
  } else {
    buf.push_str(entry.time);
    buf.push_str(" ");
    buf.push_str(level_ss);
  }

  buf.push_str(" ");
  buf.push_str(entry.msg);
  buf.push_str("\n");

  return buf.as_mut_ptr();
}

}  // namespace sfc::log
