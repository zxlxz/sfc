#pragma once

#include "sfc/alloc.h"

namespace sfc::app {

struct Arg {
  char _short;
  Str _long;
  Str _help;
  Str _default = {};
};

class Cmd {
  struct XArg;

  String _name;
  String _version;
  String _author;
  String _about;
  Vec<XArg> _args;

 public:
  Cmd(Str name) noexcept;
  ~Cmd() noexcept;

  void set_author(Str s);
  void set_version(Str s);
  void set_about(Str s);
  void add_arg(Arg arg);
  void add_flag(Arg flag);

  auto get(Str name) const -> Option<Str>;
  auto get_flag(Str name) const -> Option<bool>;
  auto get_args(Str name) const -> Slice<const String>;

  void parse(Slice<const Str> args);
  void parse_cmdline(int argc, const char* argv[]);

  void print_help() const;
};

}  // namespace sfc::app
