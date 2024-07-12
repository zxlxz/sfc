#include "file_backend.h"

#include "sfc/time.h"

namespace sfc::log {

FileBackend::FileBackend(fs::File file) : _file{mem::move(file)} {}

FileBackend::FileBackend(FileBackend&&) noexcept = default;

FileBackend::~FileBackend() {}

auto FileBackend::create(fs::Path path) -> io::Result<FileBackend> {
  auto file = fs::File::create(path);
  if (file.is_err()) {
    return mem::move(file).get_err_unchecked();
  }
  return FileBackend{mem::move(file).unwrap()};
}

void FileBackend::write_entry(Entry entry) {
  const auto log_str = this->make_log_str(entry);
  _file.write_str(log_str);
}

auto FileBackend::make_log_str(Entry entry) const -> Str {
  static thread_local auto gBuf = String{};

  static const u64 LEVEL_CNT = static_cast<u64>(Level::Fatal) + 1;
  static const Str LEVEL_STR[] = {
      " [TT] ", " [DD] ", " [II] ", " [WW] ", " [EE] ", " [XX] ", " [??] ",
  };

  const auto level_id = cmp::min(static_cast<u64>(entry.level), LEVEL_CNT);
  const auto level_ss = LEVEL_STR[level_id];

  auto& buf = gBuf;
  buf.clear();
  buf.write_str(entry.time);
  buf.write_str(level_ss);
  buf.write_str(entry.msg);
  buf.write_str("\n");
  return buf.as_str();
}

}  // namespace sfc::log
