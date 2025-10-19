#pragma once

#include "sfc/alloc.h"

namespace sfc::app {

class Clap {
  struct Item;
  struct Parser;
  String _name;
  String _version;
  String _author;
  String _about;
  Vec<Item> _items;

 public:
  Clap(Str name) noexcept;
  ~Clap() noexcept;

  void set_author(Str s);
  void set_version(Str s);
  void set_about(Str s);

  void add_opt(Str name, Str help, Str val = {});
  void add_arg(Str name, Str help);

  auto get(Str name) const -> Option<Str>;
  auto get_flag(Str name) const -> bool;
  void print_help() const;

  void parse(Slice<const Str> args);
  void parse_cmdline(int argc, const char* argv[]);
};

}  // namespace sfc::app
