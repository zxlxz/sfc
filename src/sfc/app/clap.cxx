#include "sfc/app/clap.h"

#include "sfc/test.h"

namespace sfc::app::test {

// add test for Clap
SFC_TEST(clap) {
  auto clap = Clap{"clap_test"};
  clap.opt("v:verbose", "Enable verbose output");
  clap.opt("c:config", "Configuration file");
  clap.arg("i:input...", "Input file");
  clap.arg("o:output", "Output file");

  const Str args[] = {"--verbose", "input.txt", "--output", "output.txt", "-c=config.yaml"};
  clap.parse(args);
  clap.print_help();

  panicking::expect(!clap.get("help"));
  panicking::expect(clap.get("verbose"));

  panicking::expect_eq(clap.get("input"), Option{"input.txt"});
  panicking::expect_eq(clap.get("output"), Option{"output.txt"});
  panicking::expect_eq(clap.get("config"), Option{"config.yaml"});
}

}  // namespace sfc::app::test
