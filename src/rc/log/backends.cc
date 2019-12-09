#include "rc.inl"

#include "rc/fs.h"
#include "rc/io.h"
#include "rc/term.h"
#include "rc/log/backends.h"

namespace rc::log {

struct LevelInfo {
  Str _name;
  Str _prefix;
  term::Style _style;
};

static auto get_info(Level level) -> LevelInfo {
  switch (level) {
    case Level::Trace:
      return LevelInfo{u8"Trace", u8"[::]", term::FG_DEFAULT};
    case Level::Debug:
      return LevelInfo{u8"Debug", u8"[--]", term::FG_GREEN};
    case Level::Info:
      return LevelInfo{u8"Info", u8"[**]", term::FG_CYAN};
    case Level::Warn:
      return LevelInfo{u8"Wran", u8"[??]", term::FG_YELLOW};
    case Level::Error:
      return LevelInfo{u8"Error", u8"[!!]", term::FG_RED};
    case Level::Fatal:
      return LevelInfo{u8"Fatal", u8"[XX]", term::BG_RED};
    case Level::User:
      return LevelInfo{u8"User", u8"", {}};
  }
  return LevelInfo{u8"", u8"", {}};
}

pub auto Stdout::push(const Entry& entry) -> void {
  const auto cols = term::columns();
  const auto secs = entry._time.as_secs_f64();
  const auto info = log::get_info(entry._level);

  auto buf = io::Stdout{}.lock();
  auto out = fmt::Formatter{buf};
  if (!info._prefix.is_empty()) {
    out.write(u8"{}{} \x1b[0m", info._style, info._prefix);
  }

  for (const auto& c : entry._msg) {
    out.write_chr(c);
    if (c == u8'\n') {
      out.write_str(u8"   | ");
    }
  }
  out.write(u8"\x1b[{}G\x1b[36m{>7.3}\x1b[0m\n", cols - 6, secs);
}

pub auto File::create(fs::Path p) -> File {
  return File{Inner{fs::File::create(p)}};
}

pub auto File::push(const Entry& entry) -> void {
  const auto info = log::get_info(entry._level);
  const auto secs = entry._time.as_secs_f64();

  auto out = fmt::Formatter{_inner};
  out.write(u8"{>8.3} {}", secs, info._prefix);

  for (auto c : entry._msg) {
    if (c == '\n') {
      out.write_str(u8"              ");
    }
    out.write_chr(c);
  }
  out.write_chr(u8'\n');
}

}  // namespace rc::log
