#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

void benchmark_spd(uint32_t count) {
  auto logger = spdlog::basic_logger_mt("spd", "out/spd.log");
  spdlog::set_default_logger(logger);

  spdlog::flush_on(spdlog::level::info);
  for (auto i = 0U; i < count; ++i) {
    spdlog::info("channel={}, val={}", "NIADS_COMMON_TEST_CHANNEL", 1.234);
  }
}
