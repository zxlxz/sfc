#include "sfc/app/clap.h"

#include "sfc/test/ut.h"

namespace sfc::app::test {

// add test for Clap
SFC_TEST(clap) {
  auto clap = Clap{"clap_test"};
  clap.add_flag("help", "Show help message");
  clap.add_flag("verbose", "Enable verbose output");
  clap.add_opt("c:config", "Configuration file", "CONFIG");
  clap.add_arg("i:input", "Input file", "INPUT...");
  clap.add_arg("o:output", "Output file", "OUTPUT");
  const Str args[] = {"--verbose", "--output", "output.txt", "-c=config.yaml", "a.txt", "b.txt"};
  clap.parse(args);

  panicking::expect_eq(clap.get_flag("help"), Option<bool>{});
  panicking::expect_eq(clap.get_flag("verbose"), Option{true});

  panicking::expect_eq(clap.get("input"), Option{"a.txt;b.txt"});
  panicking::expect_eq(clap.get("output"), Option{"output.txt"});
  panicking::expect_eq(clap.get("config"), Option{"config.yaml"});
}

}  // namespace sfc::app::test
