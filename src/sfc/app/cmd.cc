#include "sfc/app/cmd.h"
#include "sfc/io/stdio.h"

namespace sfc::app {

struct Cmd::Item {
  enum class Type : u8 {
    Arg,
    Opt,
  };

  Type _type;
  char _short;
  String _long;
  String _help;
  String _default = {};
  Vec<String> _values = {};

 public:
  static auto from(const Opt& t) -> Item {
    return {Type::Opt, t._short, String::from(t._long), String::from(t._help)};
  }

  static auto from(const Arg& t) -> Item {
    return {Type::Arg, t._short, String::from(t._long), String::from(t._help), String::from(t._default)};
  }

  auto match(Str key) const -> bool {
    if (key.len() == 1) {
      return key[0] == _short;
    }

    if (_long.ends_with('*')) {
      return key == _long.slice(0, key.len() - 1);
    }

    return key == _long;
  }

  auto is_complete() const -> bool {
    if (_values.is_empty() || _values[0].is_empty()) {
      return false;
    }
    if (!_long.ends_with('*')) {
      return true;
    }
    return false;
  }

  void update(Str key) {
    if (!_values.is_empty() && _values.last()->is_empty()) {
      _values.pop();
    }
    _values.push(String::from(key));
  }

  auto var() const -> Option<Str> {
    if (_values.is_empty()) {
      return {};
    }
    const auto& res = _values[0];
    return res.as_str();
  }

  auto flag() const -> Option<bool> {
    if (_values.is_empty()) {
      return {};
    }

    const auto& val = _values[0];
    if (val.is_empty() && _default.is_empty()) {
      return true;
    }

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
    if (!_default.is_empty()) {
      f.write_fmt("[default: {}]", _default);
    }
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

void Cmd::add_arg(Arg arg) {
  _items.push(Item::from(arg));
}

void Cmd::add_opt(Opt opt) {
  _items.push(Item::from(opt));
}

auto Cmd::get(Str s) const -> Option<Str> {
  for (auto& item : _items) {
    if (item.match(s)) {
      return item.var();
    }
  }
  return {};
}

auto Cmd::get_opt(Str s) const -> Option<bool> {
  for (auto& item : _items) {
    if (item.match(s)) {
      return item.flag();
    }
  }
  return {};
}

auto Cmd::get_args(Str s) const -> Slice<const String> {
  for (auto& item : _items) {
    if (item.match(s)) {
      return item._values.as_slice();
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
    return Tuple{s.slice(0, *p), s.slice(*p + 1, s.len())};
  };

  auto find_args = [this]() -> Item* {
    for (auto& item : _items) {
      if (item._type == Item::Type::Arg && !item.is_complete()) {
        return &item;
      }
    }
    return nullptr;
  };

  auto find_item = [this](Str s) -> Item* {
    for (auto& item : _items) {
      if (item.match(s)) {
        return &item;
      }
    }
    return nullptr;
  };

  auto prev_item = static_cast<Item*>(nullptr);
  for (auto arg : args) {
    if (!arg) {
      continue;
    }

    if (arg[0] == '-') {
      const auto [key, val] = parse_kv(arg);
      if ((prev_item = find_item(key))) {
        prev_item->update(val);
      }
      continue;
    }

    if (prev_item) {
      prev_item->update(arg);
      prev_item = prev_item->is_complete() ? nullptr : prev_item;
      continue;
    }

    if (auto item = find_args()) {
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
