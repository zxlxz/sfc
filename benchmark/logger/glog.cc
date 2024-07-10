#define GLOG_USE_GLOG_EXPORT
#include <glog/logging.h>

void benchmark_glog(uint32_t count) {
  google::InitGoogleLogging("glog");
  google::SetLogDestination(google::GLOG_INFO, "./out/glog");

  for (auto i = 0U; i < count; ++i) {
    LOG(INFO) << "channel=" << "NIADS_COMMON_TEST_CHANNEL, " << "val=" << 1.234;
  }
}
