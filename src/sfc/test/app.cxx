#include "sfc/app/opts.h"
#include "sfc/test.h"

using namespace sfc;

int main(int argc, const char* argv[]) {
  auto opts = app::Opts{};
  opts.add_opt("help", "Show this help message", 'h');
  opts.add_opt("list", "List all tests", 'l');

  // Add options for Google Test compatibility
  opts.add_opt("gtest_list_tests", "List all tests instead of running them");
  opts.add_opt("gtest_filter", "Run only tests matching the given pattern");
  opts.add_opt("gtest_color", "Enable or disable colored output", 0, "auto");
  opts.add_opt("gtest_output", "Output file for test results", 0, "stdout");

  // parse command line arguments
  opts.parse_cmdline(argc, argv);

  auto& app = test::App::instance();
  if (opts.has("help")) {
    app.help();
    return 0;
  }

  if (opts.has("list")) {
    app.list();
    return 0;
  }

  if (opts.has("gtest_list_tests")) {
    const auto output = opts.get("gtest_output").unwrap_or("stdout");
    app.list_xml(output);
    return 0;
  }

  const auto gtest_color = opts.get_flag("gtest_color");
  const auto gtest_filter = opts.get("gtest_filter").unwrap_or("");
  app.run(gtest_filter, gtest_color);
  return 0;
}
