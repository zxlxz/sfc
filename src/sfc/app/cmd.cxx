#include "sfc/app/cmd.h"

#include "sfc/test.h"

namespace sfc::app::test {

// add test for Cmd
SFC_TEST(cmd) {
  auto cmd = Cmd{"cmd_test"};
  cmd.add_flag({'v', "verbose", "Enable verbose output", "false"});
  cmd.add_arg({'o', "output", "Output file", "output.txt"});
  cmd.add_arg({'c', "config", "Configuration file", "config.yaml"});

  const Str args[] = {
      "--verbose=true",
      "--output",
      "output.txt",
      "-c",
      "config.yaml",
  };
  cmd.parse(args);

  panicking::assert_eq(cmd.get("verbose"), Option<Str>{"true"});
  panicking::assert_eq(cmd.get_flag("verbose"), Option{true});

  panicking::assert_eq(cmd.get("output"), Option<Str>{"output.txt"});
  panicking::assert_false(cmd.get_flag("output"));

  panicking::assert_eq(cmd.get("config"), Option<Str>{"config.yaml"});
  panicking::assert_false(cmd.get_flag("config"));
}

}  // namespace sfc::app::test
