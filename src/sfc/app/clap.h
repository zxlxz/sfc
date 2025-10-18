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

  void author(Str s);
  void version(Str s);
  void about(Str s);

  void opt(Str name, Str help);
  void arg(Str name, Str help);

  auto get(Str name) const -> Option<Str>;
  auto get_flag(Str name) const -> Option<bool>;

  void parse(Slice<const Str> args);
  void parse_cmdline(int argc, const char* argv[]);

  void print_help() const;
};

}  // namespace sfc::app
