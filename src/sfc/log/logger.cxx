#include "sfc/log.h"

#include "sfc/test/test.h"

namespace sfc::log::test {

SFC_TEST(log_funcs) {
  log::global().set_level(log::Level::Trace);

  log::trace("log {}", 1);
  log::debug("log {}", 2);
  log::warn("log {}", 3);
  log::info("log {}", 4);
  log::error("log {}", 5);
  log::fatal("log {}", 6);
}

}  // namespace sfc::log::test
