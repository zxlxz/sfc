#include "file_backend.h"

#include "sfc/time.h"

namespace sfc::log {

FileBackend::FileBackend(Str path) : _file{fs::File::create(path)} {}

FileBackend::FileBackend(FileBackend&&) noexcept = default;

FileBackend::~FileBackend() {}

static auto make_time_str() -> Str {
  static thread_local char time_buf[32];

  static const auto time_start = time::Instant::now();
  const auto time_dur = time_start.elpased().as_secs_f64();
  const auto time_len = __builtin_snprintf(time_buf, sizeof(time_buf), "[%10.6f] ", time_dur);
  if (time_len <= 0) {
    return {};
  }
  return Str{time_buf, static_cast<u64>(time_len)};
}

static auto make_level_str(Level level) -> Str {
  switch (level) {
    case Level::Trace:
      return "[TT] ";
    case Level::Debug:
      return "[DD] ";
    case Level::Info:
      return "[II] ";
    case Level::Warning:
      return "[WW] ";
    case Level::Error:
      return "[EE] ";
    case Level::Fatal:
      return "[XX] ";
  }
  return "[??] ";
}

static auto make_log_str(Level level, Str msg) -> Str {
  static thread_local String buf;
  buf.clear();

  const auto time_str = make_time_str();
  const auto level_str = make_level_str(level);

  buf.push_str(time_str);
  buf.push_str(level_str);
  buf.push_str(msg);
  buf.push_str("\n");
  return buf.as_str();
}

void FileBackend::write_msg(Level level, Str msg) {
  const auto log_str = make_log_str(level, msg);
  _file.write_str(log_str);
}

}  // namespace sfc::log
