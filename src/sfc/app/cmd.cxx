#include "sfc/app/cmd.h"

#include "sfc/test.h"

namespace sfc::app::test {

// add test for Cmd
SFC_TEST(cmd) {
  auto cmd = Cmd{"cmd_test"};
  cmd.add_opt("c:config", "Configuration file");
  cmd.add_opt("v:verbose", "Enable verbose output");
  cmd.add_arg("+i:input", "Input file");
  cmd.add_arg("+o:output", "Output file");

  const Str args[] = {"--verbose", "input.txt", "--output", "output.txt", "-c=config.yaml"};
  cmd.parse(args);
  cmd.print_help();

  panicking::expect(!cmd.get("help"));
  panicking::expect(cmd.get("verbose"));

  panicking::expect_eq(cmd.get("input"), Option{"input.txt"});
  panicking::expect_eq(cmd.get("output"), Option{"output.txt"});
  panicking::expect_eq(cmd.get("config"), Option{"config.yaml"});
}

}  // namespace sfc::app::test
