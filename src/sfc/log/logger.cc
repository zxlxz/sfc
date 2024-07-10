#include "logger.h"

#include "backend/console_backend.h"
#include "backend/file_backend.h"

namespace sfc::log {

Logger::Logger() = default;

Logger::Logger(Logger&&) noexcept = default;

Logger::~Logger() {}

Logger& Logger::operator=(Logger&&) noexcept = default;

auto Logger::get_level() const -> Level {
  return _level;
}

void Logger::set_level(Level level) {
  _level = level;
}

void Logger::write_msg(Level level, Str msg) {
  if (level < _level) {
    return;
  }

  for (auto& be : _backends.as_mut_slice()) {
    be->write_msg(level, msg);
  }
}

void Logger::clear_backends() {
  _backends.clear();
}

void Logger::add_console_backend() {
  auto backend = Box<ConsoleBackend>::xnew();
  this->add_backend(Box<IBackend&>(mem::move(backend)));
}

void Logger::add_file_backend(Str path) {
  auto backend = Box<FileBackend>::xnew(path);
  this->add_backend(Box<IBackend&>(mem::move(backend)));
}

void Logger::add_backend(Box<IBackend&> backend) {
  _backends.push(mem::move(backend));
}

auto Logger::get_tls_sbuf() -> String& {
  static thread_local auto res = String{};

  res.clear();
  return res;
}

}  // namespace sfc::log
