#include "sfc/test/app.h"
#include "sfc/app/clap.h"
#include "sfc/test/gtest.h"
#include "sfc/io/stdio.h"

namespace sfc::test {

auto main(int argc, const char* argv[]) -> int {
  auto cmd = app::Clap{"sfc_test"};
  cmd.flag("h:help", "Show this help message");
  cmd.flag("gtest_list_tests", "List all tests instead of running them");
  cmd.opt("gtest_filter", "Run only tests matching the given pattern", "PATTERNS");
  cmd.opt("gtest_color", "Enable or disable colored output", "BOOL");
  cmd.opt("gtest_output", "Output file for test results", "FILE");

  if (!cmd.parse_cmdline(argc, argv)) {
    cmd.print_help();
    return -1;
  }

  auto gtest = GTestApp{};
  if (cmd.get("help")) {
    gtest.show_help();
    return 0;
  }

  if (cmd.get("gtest_list_tests")) {
    const auto out_path = cmd.get("gtest_output").unwrap_or("stdout");
    gtest.list(out_path);
    return 0;
  }

  const auto test_filter = cmd.get("gtest_filter").unwrap_or("");
  gtest.filt(test_filter);

  const auto is_terminal = io::stdout().is_terminal();
  const auto use_color = cmd.get_flag("gtest_color").unwrap_or(is_terminal);
  const auto run_ret = gtest.run(use_color);

  return run_ret ? 0 : -1;
}

}  // namespace sfc::test
