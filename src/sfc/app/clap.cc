#include "sfc/app/clap.h"
#include "sfc/io/stdio.h"

namespace sfc::app {

struct Clap::Item {
  enum Type {
    Opt,
    Arg,
  };

  Type _type = Type::Opt;
  char _key = 0;
  Str _name = {};
  Str _help = {};
  Str _val_name = {};
  u32 _max_cnt = 1;

  bool _has_set = false;
  u32 _val_cnt = 0;
  String _value = {};

 public:
  Item(Type type, char key, Str name, Str help, Str val, u32 val_cnt)
      : _type{type}, _key{key}, _name{name}, _help{help}, _val_name{val}, _max_cnt{val_cnt} {}

  auto match(Str key) const -> bool {
    if (key.is_empty()) {
      return false;
    }
    if (key.len() == 1) {
      return key[0] == _key;
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

  auto flag() const -> Option<bool> {
    if (!_has_set) {
      return {};
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

    return {};
  }

  void show_usage(auto& f) const {
    if (_type != Type::Arg) {
      return;
    }
    f.write_fmt(" [{}]", _val_name);
    if (_max_cnt > 1) {
      f.write_str("...");
    }
  }

  void show_help(auto& f) const {
    if (_key == 0) {
      f.write_str("      --");
    } else {
      char buf[] = "  - , --";
      buf[3] = _key;
      f.write_str(buf);
    }

    f.write_str(_name);
    if (_val_name.is_empty()) {
      f.write_str("      ");
    } else {
      f.write_str(" <");
      f.write_str(_val_name);
      f.write_str(_max_cnt <= 1 ? Str{">   "} : Str{">..."});
    }

    const char pad[] = "                                ";
    const auto nwrite = 8 + _name.len() + 6 + _val_name.len();
    if (nwrite < sizeof(pad)) {
      f.write_str({pad, sizeof(pad) - nwrite});
    }
    f.write_str(_help);
  }
};

struct Clap::Parser {
  Slice<Item> _items;
  Item* _prev_item = nullptr;

 public:
  auto parse(Slice<const Str> args) -> bool {
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

    return this->parse_position_args(pos_vals.as_slice());
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

  auto parse_position_args(Slice<const Str> vals) -> int {
    const auto args_cnt = static_cast<u32>(this->position_args_cnt());
    const auto vals_cnt = static_cast<u32>(vals.len());

    if (vals_cnt < args_cnt) {
      return -static_cast<int>(args_cnt - vals_cnt);
    }

    auto iter = vals.iter();
    for (auto& item : _items) {
      if (item._type != Item::Type::Arg || item._has_set) {
        continue;
      }
      while (!item.is_complete()) {
        if (auto val = iter.next()) {
          item.push_val(*val);
        } else {
          break;
        }
      }
    }

    const auto rem_cnt = iter.count();
    return static_cast<int>(rem_cnt);
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

void Clap::add_opt(Str desc, Str help, Str sval) {
  const auto skey = desc[1] == ':' ? desc[0] : char(0);
  const auto name = skey ? desc[{2, $}] : desc;
  if (sval.is_empty()) {
    _items.push({Item::Type::Opt, skey, name, help, {}, 0});
    return;
  }

  const auto max_cnt = sval.ends_with("...") ? 1000U : 1U;
  const auto val_name = max_cnt == 1 ? sval : sval[{0, sval.len() - 3}];
  _items.push({Item::Type::Opt, skey, name, help, val_name, max_cnt});
}

void Clap::add_arg(Str desc, Str help, Str sval) {
  const auto skey = desc[1] == ':' ? desc[0] : char(0);
  const auto name = skey ? desc[{2, $}] : desc;
  const auto max_cnt = sval.ends_with("...") ? 1000U : 1U;
  const auto val_name = max_cnt == 1 ? sval : sval[{0, sval.len() - 3}];
  _items.push({Item::Type::Arg, skey, name, help, val_name, max_cnt});
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

void Clap::print_help() const {
  auto out = io::Stdout::lock();

  if (!_about.is_empty()) {
    out.write_str(_about);
    out.write_str("\n\n");
  }

  out.write_str("Usage: ");
  out.write_str(_name);
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

auto Clap::parse(Slice<const Str> args) -> int {
  auto parser = Parser{_items.as_mut_slice()};
  return parser.parse(args);
}

auto Clap::parse_cmdline(int argc, const char* argv[]) -> int {
  static constexpr auto MAX_ARGS = 256;

  Str args[MAX_ARGS] = {};
  for (auto i = 1; i < argc && i < MAX_ARGS; i++) {
    args[i - 1] = Str::from_cstr(argv[i]);
  }
  return this->parse({args, static_cast<usize>(argc - 1)});
}

}  // namespace sfc::app
