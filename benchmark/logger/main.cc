#include "sfc/io.h"
#include "sfc/time.h"

using namespace sfc;

void benchmark_sfc(u32 count);
void benchmark_spd(u32 count);
void benchmark_glog(u32 count);

void show_time(auto s, auto&& f) {
  const auto t = time::Instant::now();
  f();
  const auto d = t.elpased();
  io::println("dur[{}]: {}", s, d.as_secs_f64());
}

int main(int argc, const char* argv[]) {
  auto cnt = 1U;
  if (argc == 2) {
    cnt = Str{argv[1]}.parse<u32>().unwrap();
  }
  io::println("=== {} ===", cnt);

  benchmark_sfc(cnt);
  show_time("sfc", [&]() { benchmark_sfc(cnt); });
  show_time("spd", [&]() { benchmark_spd(cnt); });
  show_time("glog", [&]() { benchmark_glog(cnt); });
  return 0;
}
