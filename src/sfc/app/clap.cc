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
  Str _name = {};
  Str _help_msg = {};
  Str _val_name = {};

  u32 _max_cnt = 1;
  u32 _val_cnt = 0;
  bool _has_set = false;
  String _value = {};

 public:
  Item(Type type, Str desc, Str help, Str val) : _type{type}, _name{desc}, _help_msg{help}, _val_name{val} {
    if (_type == Type::Opt && _val_name.is_empty()) {
      _max_cnt = 0;
    }

    if (const auto p = _name.find(':')) {
      const auto [a, b] = _name.split_at(*p);
      _short_name = a[0];
      _name = b[{1, $}];
    }

    if (_name.ends_with("...")) {
      _max_cnt = static_cast<u32>(-1);
      _name = _name[{0, _name.len() - 3}];
    }
  }

  auto match(Str key) const -> bool {
    if (key.is_empty()) {
      return false;
    }
    if (key.len() == 1) {
      return key[0] == _short_name;
    }
    return key == _name;
  }

  auto is_complete() const -> bool {
    if (!_has_set) {
      return false;
    }
    return _val_cnt >= _max_cnt;
  }

  auto value() const -> Option<Str> {
    if (!_has_set) {
      return {};
    }
    return _value.as_str();
  }

  void push_val(Str val) {
    _has_set = true;
    if (val.is_empty()) {
      return;
    }

    _val_cnt += 1;
    if (!_value.is_empty()) {
      _value.push_str(";");
    }
    _value.push_str(val);
  }

  auto flag() const -> bool {
    if (!_has_set) {
      return false;
    }

    const auto val = _value.as_str();
    if (val.is_empty()) {
      return true;
    }

    if (val == "1" || val == "true" || val == "yes" || val == "on") {
      return true;
    }

    if (val == "0" || val == "false" || val == "no" || val == "off") {
      return false;
    }

    return false;
  }

  void show_usage(auto& f) const {
    if (_type != Type::Arg) {
      return;
    }
    f.write_fmt(" [{}]", _name);
    if (_max_cnt > 1) {
      f.write_str("...");
    }
  }

  void show_help(auto& f) const {
    if (_short_name == 0) {
      f.write_str("      --");
    } else {
      char buf[] = "  - , --";
      buf[3] = _short_name;
      f.write_str(buf);
    }

    f.write_str(_name);
    if (_val_name.is_empty()) {
      f.write_str("   ");
    } else {
      f.write_str(" <");
      f.write_str(_val_name);
      f.write_str(">");
    }

    const auto nwrite = 8 + _name.len() + 3 + _val_name.len();
    if (nwrite < 32) {
      const char pad[] = "                                ";
      f.write_str({pad, 32 - nwrite});
    }

    f.write_str(_help_msg);
  }
};

struct Clap::Parser {
  Slice<Item> _items;
  Item* _prev_item = nullptr;

 public:
  void parse(Slice<const Str> args) {
    auto pos_vals = Vec<Str>{};
    pos_vals.reserve(args.len());

    _prev_item = nullptr;
    for (auto s : args) {
      if (!s) {
        continue;
      }
      if (!this->parse_opt(s)) {
        pos_vals.push(s);
      }
    }

    this->parse_position_args(pos_vals.as_slice());
  }

 private:
  static auto parse_kv(Str s) -> Tuple<Str, Str> {
    s = s.trim_start_matches('-');

    if (auto p = s.find('=')) {
      const auto [a, b] = s.split_at(*p);
      return {a, b[{1, $}]};
    }
    return {s, {}};
  }

  auto get_item(Str s) -> Item* {
    for (auto& item : _items) {
      if (item.match(s)) {
        return &item;
      }
    }
    return nullptr;
  }

  auto parse_opt(Str s) -> bool {
    // --key=value
    if (s[0] == '-') {
      const auto [key, val] = parse_kv(s);
      if (auto item = this->get_item(key)) {
        item->push_val(val);
        _prev_item = item;
      }
      return true;
    }

    // named arg
    if (_prev_item && !_prev_item->is_complete()) {
      _prev_item->push_val(s);
      return true;
    }

    return false;
  }

  auto position_args_cnt() const -> u32 {
    auto cnt = 0U;
    for (const auto& item : _items) {
      if (item._type == Item::Type::Arg && item._has_set == false) {
        cnt += 1;
      }
    }
    return cnt;
  }

  void parse_position_args(Slice<const Str> pos_vals) {
    if (pos_vals.is_empty()) {
      return;
    }
    const auto args_cnt = this->position_args_cnt();
    const auto vals_cnt = static_cast<u32>(pos_vals.len());

    auto extra_num = vals_cnt > args_cnt ? vals_cnt - args_cnt : 0U;
    auto val_idx = 0U;
    for (auto& item : _items) {
      if (val_idx >= vals_cnt) {
        break;
      }
      if (item._type != Item::Type::Arg || item._has_set) {
        continue;
      }

      item.push_val(pos_vals[val_idx++]);
      while (extra_num > 0 && !item.is_complete()) {
        item.push_val(pos_vals[val_idx++]);
        extra_num--;
      }
    }
  }
};

Clap::Clap(Str name) noexcept : _name{String::from(name)} {}

Clap::~Clap() noexcept {}

void Clap::set_author(Str s) {
  _author = String::from(s);
}

void Clap::set_version(Str s) {
  _version = String::from(s);
}

void Clap::set_about(Str s) {
  _about = String::from(s);
}

void Clap::add_opt(Str name, Str help, Str val) {
  _items.push({Item::Type::Opt, name, help, val});
}

void Clap::add_arg(Str name, Str help) {
  _items.push({Item::Type::Arg, name, help, {}});
}

auto Clap::get(Str s) const -> Option<Str> {
  for (auto& item : _items) {
    if (item.match(s)) {
      return item.value();
    }
  }
  return {};
}

auto Clap::get_flag(Str s) const -> bool {
  for (auto& item : _items) {
    if (item.match(s)) {
      return item.flag();
    }
  }
  return false;
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

void Clap::parse(Slice<const Str> args) {
  auto parser = Parser{_items.as_mut_slice()};
  parser.parse(args);
}

void Clap::parse_cmdline(int argc, const char* argv[]) {
  static constexpr auto MAX_ARGS = 64;

  Str args[MAX_ARGS] = {};
  for (auto i = 1; i < argc && i < MAX_ARGS; i++) {
    args[i - 1] = Str::from_cstr(argv[i]);
  }
  this->parse({args, static_cast<usize>(argc - 1)});
}

}  // namespace sfc::app
