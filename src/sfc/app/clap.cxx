#include "sfc/app/clap.h"

#include "sfc/test/test.h"

namespace sfc::app::test {

// add test for Clap
SFC_TEST(clap) {
  auto clap = Clap{"clap_test"};
  clap.flag("help", "Show help message");
  clap.flag("verbose", "Enable verbose output");
  clap.opt("c:config", "Configuration file", "CONFIG");
  clap.arg("i:input", "Input file", "INPUT...");
  clap.arg("o:output", "Output file", "OUTPUT");

  const Str args[] = {"--verbose", "--output", "output.txt", "-c=config.yaml", "a.txt", "b.txt"};
  clap.parse(args);

  sfc::expect_eq(clap.get_flag("help"), Option<bool>{});
  sfc::expect_eq(clap.get_flag("verbose"), Option{true});

  sfc::expect_eq(clap.get("input"), Option{"a.txt;b.txt"});
  sfc::expect_eq(clap.get("output"), Option{"output.txt"});
  sfc::expect_eq(clap.get("config"), Option{"config.yaml"});
}

}  // namespace sfc::app::test
