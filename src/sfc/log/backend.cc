#include "sfc/log/backend.h"
#include "sfc/io/stdio.h"

namespace sfc::log {

void ConsoleBackend::push(Record record) noexcept {
  char buf[4096];
  auto out = fmt::SBuf{buf};
  fmt::Formatter{out}.write_val(record);

  const auto msg = out.as_str();
  io::Stdout().write_str(msg);
}

void ConsoleBackend::flush() noexcept {}

FileBackend::FileBackend(fs::File file) noexcept : _file{mem::move(file)} {}

FileBackend::~FileBackend() noexcept {}

void FileBackend::push(Record record) noexcept {
  char buf[4096];
  auto out = fmt::SBuf{buf};
  fmt::Formatter{out}.write_val(record);

  const auto msg = out.as_str();
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

void GlobalBackend::push(Record record) noexcept {
  char buf[4096];
  auto out = fmt::SBuf{buf};
  fmt::Formatter{out}.write_val(record);

  const auto msg = out.as_str();
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
