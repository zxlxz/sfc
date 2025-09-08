#pragma once

#include "sfc/alloc.h"

namespace sfc::app {

struct Arg {
  char _short;
  Str _long;
  Str _desc;
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

  auto author(Str s) -> Cmd&;
  auto version(Str s) -> Cmd&;
  auto about(Str s) -> Cmd&;
  auto arg(Arg arg) -> Cmd&;

  auto get(Str name) const -> Option<Str>;
  auto get_flag(Str name) const -> Option<bool>;

  auto update(Str key, Str val) -> bool;
  void parse(Slice<const Str> args);
  void parse_cmdline(int argc, const char* argv[]);

  void print_help() const;
};

}  // namespace sfc::app
