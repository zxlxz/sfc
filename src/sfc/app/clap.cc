#include "sfc/app/clap.h"
#include "sfc/io/stdio.h"

namespace sfc::app {

struct Clap::Item {
  enum Type {
    Opt,
    Arg,
  };

  Type _type;
  char _short_name = 0;
  String _long_name = {};
  String _help_msg = {};

  u32 _max_cnt = 1;
  u32 _val_cnt = 0;
  String _value = {};

 public:
  Item(Type type, Str name, Str help) : _type{type}, _help_msg{String::from(help)} {
    if (const auto p = name.find(':')) {
      const auto [a, b] = name.split_at(*p);
      _short_name = a[0];
      name = b[{1, $}];
    }

    if (name.ends_with("...")) {
      _max_cnt = static_cast<u32>(-1);
      name = name[{0, name.len() - 3}];
    }

    _long_name = String::from(name);
  }

  auto match(Str key) const -> bool {
    if (key.is_empty()) {
      return false;
    }
    if (key.len() == 1) {
      return key[0] == _short_name;
    }
    return key == _long_name;
  }

  auto is_complete() const -> bool {
    if (_value.is_empty()) {
      return false;
    }
    return _val_cnt >= _max_cnt;
  }

  auto value() const -> Option<Str> {
    if (_val_cnt == 0) {
      return {};
    }
    return _value.as_str();
  }

  void push_val(Str val) {
    if (val.is_empty()) {
      if (_value.is_empty()) {
        _val_cnt = 1;
      }
      return;
    }

    if (_value.is_empty()) {
      _val_cnt = 0;
    }

    _val_cnt += 1;
    if (!_value.is_empty()) {
      _value.push_str(";");
    }
    _value.push_str(val);
  }

  auto flag() const -> Option<bool> {
    if (_val_cnt == 0) {
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

  void show_usage(auto& f) const {
    if (_type != Item::Arg) {
      return;
    }
    f.write_fmt(" [{}]", _long_name);
    if (_max_cnt > 1) {
      f.write_str("...");
    }
  }

  void show_help(auto& f) const {
    f.write_str("  ");
    if (_short_name != 0) {
      f.write_fmt("-{}, ", _short_name);
    }
    f.write_fmt("--{:<30} {}", _long_name, _help_msg);
  }
};

static inline auto parse_kv(Str s) -> Tuple<Str, Str> {
  s = s.trim_start_matches('-');
  const auto p = s.find('=');
  if (!p) {
    return Tuple{s, Str{}};
  }
  const auto [a, b] = s.split_at(*p);
  return Tuple{a, b[{1, $}]};
}

Clap::Clap(Str name) noexcept : _name{String::from(name)} {}

Clap::~Clap() noexcept {}

void Clap::author(Str s) {
  _author = String::from(s);
}

void Clap::version(Str s) {
  _version = String::from(s);
}

void Clap::about(Str s) {
  _about = String::from(s);
}

void Clap::opt(Str name, Str help) {
  _items.push({Item::Opt, name, help});
}

void Clap::arg(Str name, Str help) {
  _items.push({Item::Arg, name, help});
}

auto Clap::get(Str s) const -> Option<Str> {
  for (auto& item : _items) {
    if (item.match(s)) {
      return item.value();
    }
  }
  return {};
}

auto Clap::get_flag(Str s) const -> Option<bool> {
  for (auto& item : _items) {
    if (item.match(s)) {
      return item.flag();
    }
  }
  return {};
}

void Clap::parse(Slice<const Str> args) {
  auto find_opt = [this](Str s) -> Item* {
    for (auto& item : _items) {
      if (item.match(s)) {
        return &item;
      }
    }
    return nullptr;
  };

  auto next_arg = [this]() -> Item* {
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

  auto prev_arg = static_cast<Item*>(nullptr);
  for (auto s : args) {
    if (!s) {
      continue;
    }

    // --key=value
    if (s[0] == '-') {
      const auto [key, val] = parse_kv(s);
      auto item = find_opt(key);
      if (item) {
        item->push_val(val);
      }
      if (item && item->_type == Item::Arg) {
        prev_arg = item;
      }
      continue;
    }

    // named arg
    if (prev_arg && !prev_arg->is_complete()) {
      prev_arg->push_val(s);
      continue;
    }

    // positional arg
    if (auto position_arg = next_arg()) {
      position_arg->push_val(s);
      prev_arg = position_arg;
      continue;
    }
  }
}

void Clap::parse_cmdline(int argc, const char* argv[]) {
  static constexpr auto MAX_ARGS = 64;

  Str args[MAX_ARGS] = {};
  for (auto i = 1; i < argc && i < MAX_ARGS; i++) {
    args[i - 1] = Str::from_cstr(argv[i]);
  }
  this->parse({args, static_cast<usize>(argc - 1)});
}

void Clap::print_help() const {
  auto out = io::Stdout::lock();

  if (!_about.is_empty()) {
    out.write_fmt("{}\n", _about);
  }

  out.write_fmt("Usage: {}", _name);
  for (auto& item : _items) {
    item.show_usage(out);
  }
  out.write_str("\n\n");

  out.write_str("Options:\n");
  for (const auto& item : _items) {
    item.show_help(out);
    out.write_str("\n");
  }
}

}  // namespace sfc::app
