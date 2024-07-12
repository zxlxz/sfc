#include "sfc/log.h"
#include "sfc/test.h"

namespace sfc::log {

SFC_TEST(console) {
  auto logger = Logger{};
  logger.set_level(Level::Trace);
  logger.add_backend(log::ConsoleBackend{});

  logger.write_fmt(Level::Trace, "log {}", 1);
  logger.write_fmt(Level::Debug, "log {}", 2);
  logger.write_fmt(Level::Warning, "log {}", 3);
  logger.write_fmt(Level::Info, "log {}", 4);
  logger.write_fmt(Level::Error, "log {}", 5);
  logger.write_fmt(Level::Fatal, "log {}", 6);
}

}  // namespace sfc::log
