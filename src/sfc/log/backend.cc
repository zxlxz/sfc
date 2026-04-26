#include "sfc/log/backend.h"
#include "sfc/core/fmt.h"

namespace sfc::log {

void ConsoleBackend::push(Record record) noexcept {
  io::println("{} [{}] {}", record.time_str(), record.level_str(), record.message);
}

void ConsoleBackend::flush() noexcept {}

FileBackend::FileBackend(fs::File file) noexcept : _file{mem::move(file)} {}

FileBackend::~FileBackend() noexcept {}

void FileBackend::push(Record record) noexcept {
  auto buf = fmt::FixedBuf<1024>{};
  fmt::write(buf, "{} [{}] {}\n", record.time_str(), record.level_str(), record.message);
  (void)_file.write_str(buf.as_str());
}

void FileBackend::flush() noexcept {
  (void)_file.flush();
}

GlobalBackend::GlobalBackend() noexcept {}

GlobalBackend::~GlobalBackend() noexcept {}

void GlobalBackend::set_file(fs::File file) noexcept {
  _file = mem::move(file);
}

void GlobalBackend::push(Record record) noexcept {
  auto buf = fmt::FixedBuf<1024>{};
  fmt::write(buf, "{} [{}] {}\n", record.time_str(), record.level_str(), record.message);

  if (_file.is_valid()) {
    (void)_file.write_str(buf.as_str());
  } else {
    io::Stdout::write_str(buf.as_str());
  }
}

void GlobalBackend::flush() noexcept {
  (void)_file.flush();
}

auto global() -> Logger<GlobalBackend>& {
  static auto backend = GlobalBackend{};
  static auto logger = Logger<GlobalBackend>{backend};
  return logger;
}

}  // namespace sfc::log
