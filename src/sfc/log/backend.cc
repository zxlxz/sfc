#include "sfc/log/backend.h"
#include "sfc/log/logger.h"
#include "sfc/io/stdio.h"

namespace sfc::log {

void ConsoleBackend::write(Str time_str, Str level_str, const fmt::Args& args) noexcept {
  char buf[4096];
  auto out = Slice{buf};
  fmt::write(out, "{} [{}] {}\n", time_str, level_str, args);

  const auto msg = Str{buf, sizeof(buf) - out.len()};
  io::Stdout().write_str(msg);
}

void ConsoleBackend::flush() noexcept {}

FileBackend::FileBackend(fs::File file) noexcept : _file{mem::move(file)} {}

FileBackend::~FileBackend() noexcept {}

void FileBackend::write(Str time_str, Str level_str, const fmt::Args& args) noexcept {
  char buf[4096];
  auto out = Slice{buf};
  fmt::write(out, "{} [{}] {}\n", time_str, level_str, args);

  const auto msg = Str{buf, sizeof(buf) - out.len()};
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

void GlobalBackend::write(Str time_str, Str level_str, const fmt::Args& args) noexcept {
  char buf[4096];
  auto out = Slice{buf};
  fmt::write(out, "{} [{}] {}\n", time_str, level_str, args);

  const auto msg = Str{buf, sizeof(buf) - out.len()};
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
