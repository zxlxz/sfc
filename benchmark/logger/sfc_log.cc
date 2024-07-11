#include "sfc/log.h"

using namespace sfc;

void benchmark_sfc(u32 count) {
  auto logger = log::Logger();
  logger.add_file_backend("./out/sfc.log");

  for (auto i = 0U; i < count; ++i) {
    logger.write_fmt(log::Level::Info, "channel=NIADS_COMMON_TEST_CHANNEL, val=1.234");
  }
}
