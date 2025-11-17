#include "sfc/app/clap.h"
#include "sfc/io/stdio.h"

namespace sfc::app {

struct Clap::Item {
  enum Type {
    Flag,
    Opt,
    Arg,
  };

  Type _type = Type::Opt;
  char _short_name = 0;
  Str _long_name = {};
  Str _help = {};
  Str _hint = {};
  bool _variadic = false;

  String _vals = {};
  bool _is_set = false;

 public:
  Item(Type type, Str desc, Str help, Str hint) : _type{type}, _long_name{desc}, _help{help}, _hint{hint} {
    if (desc.len() == 1) {
      _short_name = desc[0];
      _long_name = {};
    } else if (desc.len() > 1 && desc[1] == ':') {
      _short_name = desc[0];
      _long_name = desc[{2, $}];
    }

    if (hint.ends_with("...")) {
      _hint = hint[{0, hint.len() - 3}];
      _variadic = true;
    }
  }

  auto match(Str key) const -> bool {
    if (key.is_empty()) {
      return false;
    }
    if (key.len() == 1) {
      return _short_name == key[0];
    }
    return _long_name == key;
  }

  auto is_complete() const -> bool {
    if (!_is_set) {
      return false;
    }
    if (_type == Type::Flag) {
      return true;
    }
    if (_vals.is_empty()) {
      return false;
    }
    return !_variadic;
  }

  auto value() const -> Option<Str> {
    if (!_is_set) {
      return {};
    }
    return _vals.as_str();
  }

  auto flag() const -> Option<bool> {
    if (!_is_set) {
      return {};
    }

    const auto val = _vals.as_str();
    if (val.is_empty()) {
      if (_type == Type::Flag) {
        return true;
      }
      return {};
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
    if (_type == Type::Arg) {
      f.write_fmt(" [{}]{}", _hint, _variadic ? Str{"..."} : Str{""});
    }
  }

  void show_help(auto& f) const {
    static constexpr auto NAME_LEN = 32;
    auto line_buf = String::with_capacity(128);

    line_buf.push_str("  ");
    if (_short_name == 0) {
      line_buf.push_str("   ");
    } else {
      char sbuf[] = {'-', _short_name, ','};
      line_buf.push_str({sbuf, sizeof(sbuf)});
    }
    line_buf.push_str(" --");
    line_buf.push_str(_long_name);

    if (!_hint.is_empty()) {
      line_buf.push_str(" <");
      line_buf.push_str(_hint);
      line_buf.push_str(">");
      if (_variadic) {
        line_buf.push_str("...");
      }
    }

    while (line_buf.len() < NAME_LEN) {
      line_buf.push(' ');
    }

    f.write_str(line_buf);
    f.write_str(_help);
  }

  void push_val(Str val) {
    _is_set = true;
    if (val.is_empty()) {
      return;
    }

    if (!_vals.is_empty()) {
      _vals.push_str(";");
    }
    _vals.push_str(val);
  }

  auto push_vals(Slice<const Str> args) -> Slice<const Str> {
    if (_type != Type::Arg || _is_set) {
      return args;
    }

    auto cnt = 0U;
    for (auto arg : args) {
      this->push_val(arg);
      cnt += 1;
      if (this->is_complete()) {
        break;
      }
    }

    return args[{cnt, $}];
  }
};

struct Clap::Parser {
  Slice<Item> _items;
  Item* _prev_item = nullptr;

 public:
  auto parse(Slice<const Str> args) -> int {
    auto pos_vals = Vec<Str>{};

    _prev_item = nullptr;
    for (auto s : args) {
      if (!s) {
        continue;
      }

      // --key=val
      if (s[0] == '-') {
        const auto [key, val] = parse_kv(s);
        if (auto item = this->get_item(key)) {
          item->push_val(val);
          _prev_item = item;
        }
        continue;
      }

      // val
      if (_prev_item && !_prev_item->is_complete()) {
        _prev_item->push_val(s);
        continue;
      }

      pos_vals.push(s);
    }

    return this->parse_arg(pos_vals.as_slice());
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

  auto parse_arg(Slice<const Str> vals) -> int {
    auto unset_cnt = 0;

    for (auto& item : _items) {
      if (item._type != Item::Type::Arg || item._is_set) {
        continue;
      }
      if (vals.is_empty()) {
        unset_cnt += 1;
        continue;
      }
      vals = item.push_vals(vals);
    }

    if (unset_cnt != 0) {
      return -unset_cnt;
    }

    if (auto vals_cnt = vals.len()) {
      return -static_cast<int>(vals_cnt);
    }

    return 0;
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

void Clap::add_flag(Str desc, Str help) {
  _items.push({Item::Flag, desc, help, {}});
}

void Clap::add_opt(Str desc, Str help, Str sval) {
  _items.push({Item::Opt, desc, help, sval});
}

void Clap::add_arg(Str desc, Str help, Str sval) {
  _items.push({Item::Arg, desc, help, sval});
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
