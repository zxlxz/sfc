#include "sfc/app/cmd.h"

#include "sfc/test.h"

namespace sfc::app::test {

// add test for Cmd
SFC_TEST(cmd) {
  auto cmd = Cmd{"cmd_test"};
  cmd.add_arg({'c', "config", "Configuration file", "config.yaml"});
  cmd.add_arg({'o', "output", "Output file", "output.txt"});
  cmd.add_opt({'v', "verbose", "Enable verbose output"});

  const Str args[] = {
      "--verbose=true",
      "--output",
      "output.txt",
      "-c",
      "config.yaml",
  };
  cmd.parse(args);

  panicking::assert_eq(cmd.get("verbose"), Option<Str>{"true"});
  panicking::assert_eq(cmd.get_opt("verbose"), Option{true});

  panicking::assert_eq(cmd.get("output"), Option<Str>{"output.txt"});
  panicking::assert_eq(cmd.get("config"), Option<Str>{"config.yaml"});
}

}  // namespace sfc::app::test
