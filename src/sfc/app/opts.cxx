#include "sfc/app/opts.h"

#include "sfc/test.h"

namespace sfc::app::test {

// add test for Opts
SFC_TEST(opts) {
  auto opts = Opts{};
  opts.add_opt("verbose", "Enable verbose output", 'v', "false");
  opts.add_opt("output", "Output file", 'o');
  opts.add_opt("config", "Configuration file", 'c');

  const Str args[] = {
      "--verbose=true",
      "--output",
      "output.txt",
      "-c",
      "config.yaml",
  };
  opts.parse_args(args);

  panicking::assert_true(opts.has("verbose"));
  panicking::assert_eq(opts.get("verbose"), Option<Str>{"true"});
  panicking::assert_eq(opts.get_flag("verbose"), Option{true});

  panicking::assert_true(opts.has("output"));
  panicking::assert_eq(opts.get("output"), Option<Str>{"output.txt"});
  panicking::assert_false(opts.get_flag("output"));

  panicking::assert_true(opts.has("config"));
  panicking::assert_eq(opts.get("config"), Option<Str>{"config.yaml"});
  panicking::assert_false(opts.get_flag("config"));
}

}  // namespace sfc::app::test
