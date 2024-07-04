#pragma once

#include "sfc/alloc.h"

namespace sfc::app {

class Opts {
  struct Inn;
  Box<Inn> _inn;

 public:
  Opts();
  ~Opts();

  void parse_args(Slice<const Str> args);
  void parse_cmdline(int argc, const char* argv[]);

  void add_opt(Str name, Str desc, char short_name = 0, Str default_value = {});

  auto has(Str name) const -> bool;
  auto get(Str name) const -> Option<Str>;
  auto get_flag(Str name) const -> Option<bool>;
  auto usage() const -> String;
};

}  // namespace sfc::app
