#pragma once

#include "sfc/alloc.h"

namespace sfc::app {

class Clap {
  struct Item;
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
  void print_help() const;

  void add_flag(Str name, Str help);
  void add_opt(Str name, Str help, Str val);
  void add_arg(Str name, Str help, Str val);

 public:
  auto get(Str name) const -> Option<Str>;
  auto get_flag(Str name) const -> Option<bool>;

 public:
  struct Parser;
  auto parse(Slice<const Str> args) -> bool;
  auto parse_cmdline(int argc, const char* argv[]) -> bool;
};

}  // namespace sfc::app
