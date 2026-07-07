#include "sfc/test/app.h"
#include "sfc/app/clap.h"
#include "sfc/test/gtest.h"
#include "sfc/io/stdio.h"

namespace sfc::test {

void App::help() {
  test::gtest_help();
}

void App::exec(Str filter, bool use_color) {
  test::gtest_run(filter, use_color);
}

void App::list() const {
  test::gtest_list_tests({});
}

void App::list_xml(Str path) const {
  test::gtest_list_tests(path);
}

auto main(int argc, const char* argv[]) -> int {
  auto cmd = app::Clap{"sfc_test"};
  cmd.flag("h:help", "Show this help message");
  cmd.flag("l:list", "List all tests");
  cmd.flag("gtest_list_tests", "List all tests instead of running them");
  cmd.opt("gtest_filter", "Run only tests matching the given pattern", "PATTERNS");
  cmd.opt("gtest_color", "Enable or disable colored output", "BOOL");
  cmd.opt("gtest_output", "Output file for test results", "FILE");

  if (!cmd.parse_cmdline(argc, argv)) {
    cmd.print_help();
    return -1;
  }

  auto app = App{};
  if (cmd.get("help")) {
    app.help();
    return 0;
  }

  if (cmd.get("list")) {
    app.list();
    return 0;
  }

  if (cmd.get("gtest_list_tests")) {
    const auto output = cmd.get("gtest_output").unwrap_or("stdout");
    app.list_xml(output);
    return 0;
  }

  const auto is_terminal = io::Stdout{}.is_terminal();
  const auto gtest_color = cmd.get_flag("gtest_color").unwrap_or(is_terminal);
  const auto gtest_filter = cmd.get("gtest_filter").unwrap_or("");
  app.exec(gtest_filter, gtest_color);
  return 0;
}

}  // namespace sfc::test
