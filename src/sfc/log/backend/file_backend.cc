#include "file_backend.h"

namespace sfc::log {

static auto get_level_str(Level level) -> Str {
  static const u64 LEVEL_CNT = static_cast<u64>(Level::Fatal) + 1;
  static const Str LEVEL_STR[]
      = {" [VB] ", " [DE] ", " [II] ", " [WW] ", " [EE] ", " [FA] ", " [??] "};

  const auto level_id = cmp::min(static_cast<u64>(level), LEVEL_CNT);
  const auto level_ss = LEVEL_STR[level_id];
  return level_ss;
};

static auto make_log_str(Entry entry) -> Str {
  static thread_local auto buf = String();

  const auto level_str = get_level_str(entry.level);

  buf.clear();
  buf.write_str(entry.time);
  buf.write_str(level_str);
  buf.write_str(entry.msg);
  buf.write_str("\n");
  return buf.as_str();
}

FileBackend::FileBackend(fs::File file) : _file{mem::move(file)} {}

FileBackend::FileBackend(FileBackend&&) noexcept = default;

FileBackend::~FileBackend() {}

auto FileBackend::create(fs::Path path) -> io::Result<FileBackend> {
  auto opts = fs::OpenOptions{};
  opts._append = true;
  opts._write = true;
  opts._create = true;

  auto file = opts.open(path);
  if (file.is_err()) {
    return mem::move(file).get_err_unchecked();
  }
  return FileBackend{mem::move(file).unwrap()};
}

void FileBackend::flush() {}

void FileBackend::write_entry(Entry entry) {
  const auto log_str = make_log_str(entry);
  _file.write_str(log_str);
}

}  // namespace sfc::log
