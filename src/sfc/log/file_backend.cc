#include "sfc/log/file_backend.h"

namespace sfc::log {

static inline auto level_str(Level level) -> Str {
  switch (level) {
    case Level::Trace:
      return "[--] ";
    case Level::Debug:
      return "[DD] ";
    case Level::Info:
      return "[II] ";
    case Level::Warn:
      return "[WW] ";
    case Level::Error:
      return "[EE] ";
    case Level::Fatal:
      return "[!!] ";
    default:
      return "[??] ";
  }
}

FileBackend::FileBackend(fs::File&& file) : _file{mem::move(file)} {}

FileBackend::~FileBackend() noexcept {}

auto FileBackend::create(fs::Path path) -> io::Result<FileBackend> {
  const auto opts = fs::OpenOptions{.append = true, .create = true, .write = true};
  return opts.open(path).map([](fs::File file) {
    return FileBackend{mem::move(file)};
  });
}

void FileBackend::flush() {}

void FileBackend::write(Record entry) {
  static thread_local auto buf = String();

  buf.clear();
  buf.push_str(entry.time);
  buf.push_str(log::level_str(entry.level));
  buf.push_str(entry.msg);
  buf.push_str("\n");
  _file.write_str(buf.as_str()).unwrap();
}

}  // namespace sfc::log
