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
  const auto s = _enable_color ? this->make_color_log(entry) : this->make_plain_log(entry);
  io::Stdout::instance().write_str(s);
}

auto ConsoleBackend::make_plain_log(Entry entry) const -> Str {
  static thread_local auto gBuf = String{};

  static const u64 LEVEL_CNT = static_cast<u64>(Level::Fatal) + 1;

  static const Str LEVEL_STR[] = {
      "[TT]", "[DD]", "[II]", "[WW]", "[EE]", "[XX]", "[??]",
  };

  const auto level_id = cmp::min(static_cast<u64>(entry.level), LEVEL_CNT);
  const auto level_ss = LEVEL_STR[level_id];

  auto& buf = gBuf;
  buf.clear();
  buf.write_str(entry.time);
  buf.write_str(" ");
  buf.write_str(level_ss);
  buf.write_str(" ");
  buf.write_str(entry.msg);
  buf.write_str("\n");
  return buf.as_str();
}

auto ConsoleBackend::make_color_log(Entry entry) const -> Str {
  static thread_local auto gBuf = String{};

  static const u64 LEVEL_CNT = static_cast<u64>(Level::Fatal) + 1;

  static const Str COLOR_OFF = "\033[39;49m";
  static const Str COLOR_STR[] = {
      "\033[39m", "\033[32m", "\033[34m", "\033[33m", "\033[31m", "\033[41m", "",
  };

  const auto level_id = cmp::min(static_cast<u64>(entry.level), LEVEL_CNT);
  const auto color_ss = COLOR_STR[level_id];

  auto& buf = gBuf;
  buf.clear();
  buf.write_str(color_ss);
  buf.write_str(entry.time);
  buf.write_str(COLOR_OFF);
  buf.write_str(" ");
  buf.write_str(entry.msg);
  buf.write_str("\n");
  return buf.as_str();
}

}  // namespace sfc::log
