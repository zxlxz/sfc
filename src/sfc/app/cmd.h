#pragma once

#include "sfc/alloc.h"

namespace sfc::app {

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

  void add_opt(Str name, Str help);
  void add_arg(Str name, Str help);

  auto get(Str name) const -> Option<Str>;
  auto get_flag(Str name) const -> Option<bool>;

  void parse(Slice<const Str> args);
  void parse_cmdline(int argc, const char* argv[]);

  void print_help() const;
};

}  // namespace sfc::app
