#pragma once

#include "sfc/alloc.h"

namespace sfc::app {

class Opts {
  struct Item {
    char short_name{0};
    String name{};
    String desc{};
    String value{};
    String default_value{};
  };
  vec::Vec<Item> _items;

 public:
  Opts();
  ~Opts();

  void add_opt(Str name, Str desc, char short_name = 0, Str default_value = {});

  auto get(Str name) const -> Str;
  auto has_flag(Str name) const -> bool;
  auto usage() const -> String;

  void parse(Slice<const Str> args);
  auto update(Str name, Str value) -> bool;
};

}  // namespace sfc::app
