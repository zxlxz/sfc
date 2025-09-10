#pragma once

#include "sfc/alloc.h"

namespace sfc::app {

struct Opt {
  char _short;
  Str _long;
  Str _help;
};

struct Arg {
  char _short;
  Str _long;
  Str _help;
  Str _default = {};
};

class Cmd {
  struct Item;

  String _name;
  String _version;
  String _author;
  String _about;
  Vec<Item> _items;

 public:
  Cmd(Str name) noexcept;
  ~Cmd() noexcept;

  void set_author(Str s);
  void set_version(Str s);
  void set_about(Str s);
  void add_arg(Arg arg);
  void add_opt(Opt opt);

  auto get(Str name) const -> Option<Str>;
  auto get_opt(Str name) const -> Option<bool>;
  auto get_args(Str name) const -> Slice<const String>;

  void parse(Slice<const Str> args);
  void parse_cmdline(int argc, const char* argv[]);

  void print_help() const;
};

}  // namespace sfc::app
