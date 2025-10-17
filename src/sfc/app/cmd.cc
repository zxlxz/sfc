#include "sfc/app/cmd.h"
#include "sfc/io/stdio.h"

namespace sfc::app {

struct Cmd::Item {
  enum Type {
    Opt,
    Arg,
  };

  Type _type;
  char _short = 0;
  String _long = {};
  String _help = {};
  bool _has_value = false;
  String _value = {};

 public:
  Item(Type type, Str name, Str help) : _type{type}, _help{String::from(help)} {
    const auto p = name.find(':');
    if (!p) {
      _long = String::from(name);
    } else {
      const auto [a, b] = name.split_at(*p);
      _short = a[0];
      _long = String::from(b[{1, $}]);
    }
  }

  auto is_complete() const -> bool {
    if (_value.is_empty()) {
      return false;
    }
    return true;
  }

  auto match(Str key) const -> bool {
    if (key.is_empty()) {
      return false;
    }
    if (key.len() == 1) {
      return key[0] == _short;
    }
    return key == _long;
  }

  void update(Str key) {
    _has_value = true;
    if (key.is_empty()) {
      return;
    }
    if (!_value.is_empty()) {
      _value.push_str(";");
    }
    _value.push_str(key);
  }

  auto var() const -> Option<Str> {
    if (!_has_value) {
      return {};
    }
    return _value.as_str();
  }

  auto flag() const -> Option<bool> {
    if (!_has_value) {
      return {};
    }

    const auto val = _value.as_str();
    if (val == "1" || val == "true" || val == "yes" || val == "on") {
      return true;
    }

    if (val == "0" || val == "false" || val == "no" || val == "off") {
      return false;
    }

    return {};
  }

  void fmt(auto& f) const {
    if (_short != 0) {
      f.write_fmt("-{}, ", _short);
    }
    f.write_fmt("--{:<32}", _long);
  }
};

Cmd::Cmd(Str name) noexcept : _name{String::from(name)} {}

Cmd::~Cmd() noexcept {}

void Cmd::set_author(Str s) {
  _author = String::from(s);
}

void Cmd::set_version(Str s) {
  _version = String::from(s);
}

void Cmd::set_about(Str s) {
  _about = String::from(s);
}

void Cmd::add_opt(Str name, Str help) {
  _items.push({Item::Opt, name, help});
}

void Cmd::add_arg(Str name, Str help) {
  _items.push({Item::Arg, name, help});
}

auto Cmd::get(Str s) const -> Option<Str> {
  for (auto& item : _items) {
    if (item._has_value && item.match(s)) {
      return item._value.as_str();
    }
  }
  return {};
}

auto Cmd::get_flag(Str s) const -> Option<bool> {
  for (auto& item : _items) {
    if (item._has_value && item.match(s)) {
      return item.flag();
    }
  }
  return {};
}

void Cmd::parse(Slice<const Str> args) {
  auto parse_kv = [](Str s) -> Tuple<Str, Str> {
    s = s.trim_start_matches('-');
    const auto p = s.find('=');
    if (!p) {
      return Tuple{s, Str{}};
    }
    const auto [a, b] = s.split_at(*p);
    return Tuple{a, b[{1, $}]};
  };

  auto find_opt = [this](Str s) -> Item* {
    for (auto& item : _items) {
      if (item.match(s)) {
        return &item;
      }
    }
    return nullptr;
  };

  auto find_arg = [this]() -> Item* {
    auto res = static_cast<Item*>(nullptr);
    for (auto& item : _items) {
      if (item._type != Item::Arg) {
        continue;
      }
      res = &item;
      if (!item.is_complete()) {
        break;
      }
    }
    return res;
  };

  auto prev_item = static_cast<Item*>(nullptr);

  for (auto arg : args) {
    if (!arg) {
      continue;
    }

    // --key=value
    if (arg[0] == '-') {
      const auto [key, val] = parse_kv(arg);
      auto item = find_opt(key);
      if (item) {
        item->update(val);
      }
      prev_item = item;
      continue;
    }

    // arg
    if (prev_item && prev_item->_type == Item::Arg) {
      prev_item->update(arg);
      continue;
    }

    // arg
    if (auto item = find_arg()) {
      item->update(arg);
      continue;
    }
  }
}

void Cmd::parse_cmdline(int argc, const char* argv[]) {
  static constexpr auto MAX_ARGS = 64;

  Str args[MAX_ARGS] = {};
  for (auto i = 1; i < argc && i < MAX_ARGS; i++) {
    args[i - 1] = Str::from_cstr(argv[i]);
  }
  this->parse({args, static_cast<usize>(argc - 1)});
}

void Cmd::print_help() const {
  if (!_about.is_empty()) {
    io::println("{}\n", _about);
  }
  io::println("Usage: {}\n", _name);

  io::println("Options:");
  for (const auto& item : _items) {
    io::println("    {}", item);
  }
}

}  // namespace sfc::app
