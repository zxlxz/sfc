#include "sfc/log/backend.h"
#include "sfc/log/logger.h"
#include "sfc/io/stdio.h"

namespace sfc::log {

namespace detail {
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
  static thread_local auto buf = fmt::Buf<32>{};
  static thread_local auto prev_sec = u64{0};

  const auto secs = time.as_secs();
  const auto millis = time.subsec_millis();

  // write seconds, only when the seconds changed
  if (secs != prev_sec) {
    prev_sec = secs;
    buf.truncate(0);

    const auto t = time::DateTime::from_local(time);
    fmt::write(buf, "{04}-{02}-{02} {02}:{02}:{02}.000", t.year, t.month, t.day, t.hour, t.minute, t.second);
  }

  if (millis != 0 && buf.len() > 3) {
    buf.truncate(buf.len() - 3);
    fmt::write(buf, "{03}", millis);
  }

  return buf.as_str();
}

static auto write_record(auto& out, const Record& record) -> Str {
  const auto time_str = detail::time_str(record._time);
  const auto level_str = detail::level_str(record._level);
  fmt::write(out, "{} {} {}\n", time_str, level_str, record._args);
  return out.as_str();
}

}  // namespace detail

void ConsoleBackend::write(const Record& record) noexcept {
  auto buf = fmt::Buf<4096>{};
  auto msg = detail::write_record(buf, record);
  io::Stdout().write_str(msg);
}

void ConsoleBackend::flush() noexcept {}

FileBackend::FileBackend(fs::File file) noexcept : _file{mem::move(file)} {}

FileBackend::~FileBackend() noexcept {}

void FileBackend::write(const Record& record) noexcept {
  auto buf = fmt::Buf<4096>{};
  auto msg = detail::write_record(buf, record);
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
  auto buf = fmt::Buf<4096>{};
  auto msg = detail::write_record(buf, record);

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
