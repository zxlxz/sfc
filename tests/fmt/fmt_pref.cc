#include <stdio.h>
#include <format>
#include <sfc/io.h>
#include <sfc/time.h>
#include <sfc/test.h>

using namespace sfc;

SFC_TEST(c_sprintf_test) {
  const auto kMaxLoop = 1000000U;
  char buf[256U] = {};
  for (auto loop = 0U; loop < kMaxLoop; ++loop) {
    snprintf(buf, 256, "%s: int_val=%d, float_val=%.2f", "c_sprintf_test", loop, double(loop) * 0.01);
  }
  io::println("buf = {}", Str::from_cstr(buf));
}

SFC_TEST(sfc_fmt_test) {
  const auto kMaxLoop = 1000000U;
  char buf[256U] = {};
  for (auto loop = 0U; loop < kMaxLoop; ++loop) {
    auto out = fmt::SBuf{buf};
    fmt::write(out, "{}: int_val={}, float_val={:.2f}", "sfc_fmt_test", loop, double(loop) * 0.01);
    buf[out._len] = 0;
  }
  io::println("buf = {}", Str::from_cstr(buf));
}

SFC_TEST(cxx_format_test) {
  const auto kMaxLoop = 1000000U;
  char buf[256U] = {};
  for (auto loop = 0U; loop < kMaxLoop; ++loop) {
    std::format_to(buf, "{}: int_val={}, float_val={:.2f}", "cxx_format_test", loop, double(loop) * 0.01);
  }
  io::println("buf = {}", Str::from_cstr(buf));
}

int main(int argc, const char* argv[]) {
  test::main(argc, argv);
  return 0;
}
