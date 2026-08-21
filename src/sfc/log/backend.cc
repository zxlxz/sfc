#include "sfc/log/backend.h"
#include "sfc/log/logger.h"
#include "sfc/io/stdio.h"

namespace sfc::log {

static auto level_str(Level level) -> Str {
  switch (level) {
    case Level::Trace: return "[TRACE]";
    case Level::Debug: return "[DEBUG]";
    case Level::Info:  return "[INFO] ";
    case Level::Warn:  return "[WARN] ";
    case Level::Error: return "[ERROR]";
    case Level::Fatal: return "[FATAL]";
  }
  return "[INFO] ";
}

static auto time_str(time::SystemTime time) -> Str {
  static thread_local char buf[32];
  static thread_local auto out = Slice{buf};

  // write seconds, only when the seconds changed
  static thread_local auto prev_sec = u64{0};
  if (auto secs = time.as_secs(); secs != prev_sec) {
    prev_sec = secs;
    const auto t = time::DateTime::from_local(time);
    fmt::write(out, "{04}-{02}-{02} {02}:{02}:{02}.000", t.year, t.month, t.day, t.hour, t.minute, t.second);
  }

  if (auto millis = time.subsec_millis(); millis != 0) {
    out._ptr -= 3;
    out._len += 3;
    fmt::write(out, "{03}", millis);
  }

  return Str{buf, sizeof(buf) - out.len()};
}

static auto format_record(Slice<char> buf, const Record& record) -> Str {
  auto out = Slice{buf};

  const auto time_str = log::time_str(record._time);
  const auto level_str = log::level_str(record._level);
  fmt::write(out, "{} {} {}\n", time_str, level_str, record._args);
  return Str{buf._ptr, buf._len - out._len};
}

void ConsoleBackend::write(const Record& record) noexcept {
  char buf[4096];
  auto msg = log::format_record(buf, record);
  io::Stdout().write_str(msg);
}

void ConsoleBackend::flush() noexcept {}

FileBackend::FileBackend(fs::File file) noexcept : _file{mem::move(file)} {}

FileBackend::~FileBackend() noexcept {}

void FileBackend::write(const Record& record) noexcept {
  char buf[4096];
  auto msg = log::format_record(buf, record);
  (void)_file.write_str(msg);
}

void FileBackend::flush() noexcept {
  (void)_file.flush();
}

GlobalBackend::GlobalBackend() noexcept {}

GlobalBackend::~GlobalBackend() noexcept {}

void GlobalBackend::set_file(fs::File file) noexcept {
  _file = mem::move(file);
}

void GlobalBackend::write(const Record& record) noexcept {
  char buf[4096];
  auto msg = log::format_record(buf, record);

  if (_file.is_valid()) {
    (void)_file.write_str(msg);
  } else {
    io::Stdout().write_str(msg);
  }
}

void GlobalBackend::flush() noexcept {
  (void)_file.flush();
}

auto global() -> Logger& {
  static auto backend = GlobalBackend{};
  static auto logger = Logger{backend};
  return logger;
}

}  // namespace sfc::log
