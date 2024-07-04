#include "sfc/log/backend/file.h"

namespace sfc::log {

static inline auto level_str(Level level) -> Str {
  switch (level) {
    case Level::Trace:   return "[--] ";
    case Level::Debug:   return "[DD] ";
    case Level::Info:    return "[II] ";
    case Level::Warning: return "[WW] ";
    case Level::Error:   return "[EE] ";
    case Level::Fatal:   return "[!!] ";
    default:             return "[??] ";
  }
}

FileBackend::FileBackend(fs::File&& file) : _file{mem::move(file)} {}

FileBackend::~FileBackend() {}

auto FileBackend::create(fs::Path path) -> io::Result<FileBackend> {
  const auto opts = fs::OpenOptions{.append = true, .create = true, .write = true};
  auto file = opts.open(path);
  if (file.is_err()) {
    return ~file;
  }
  return FileBackend{mem::move(file).unwrap()};
}

void FileBackend::flush() {}

void FileBackend::write_entry(Entry entry) {
  static thread_local auto buf = String();

  buf.clear();
  buf.write_str(entry.time);
  buf.write_str(level_str(entry.level));
  buf.write_str(entry.msg);
  buf.write_str("\n");

  _file.write_str(buf.as_str());
}

}  // namespace sfc::log
