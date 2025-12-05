#include "sfc/app/clap.h"
#include "sfc/io/stdio.h"

namespace sfc::app {

static auto yes_or_no(Str s) -> Option<bool> {
  if (s.is_empty()) {
    return {};
  }

  if (s == "1" || s == "true" || s == "yes" || s == "on") {
    return true;
  }

  if (s == "0" || s == "false" || s == "no" || s == "off") {
    return false;
  }

  return {};
}

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
    } else if (desc[1] == ':') {
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
    if (_type == Type::Flag) {
      return true;
    }
    if (!_is_set || _vals.is_empty()) {
      return false;
    }
    return !_variadic;
  }

  auto value() const -> Option<Str> {
    if (!_is_set) {
      return {};
    }
    if (_vals.is_empty() && _type == Type::Flag) {
      return Str{"1"};
    }
    return _vals.as_str();
  }

  void show_usage(auto& f) const {
    if (_type == Type::Arg) {
      f.write_fmt(" [{}]{}", _hint, _variadic ? Str{"..."} : Str{""});
    }
  }

  void show_help(auto& f) const {
    static constexpr auto NAME_LEN = 48U;
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

 public:
  auto parse(Slice<const Str> ss) -> bool {
    const auto v1 = this->parse_opts(ss);
    const auto v2 = this->parse_args(v1.as_slice());
    return v2.is_empty();
  }

  auto unuset_args_cnt() const -> usize {
    auto accum = 0U;
    for (auto& item : _items) {
      if (item._type != Item::Arg) {
        continue;
      }
      accum += item.value() ? 0 : 1;
    }
    return accum;
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

  auto find_item(Str key) -> Item* {
    for (auto& item : _items) {
      if (item.match(key)) {
        return &item;
      }
    }
    return nullptr;
  }

  auto parse_opt(Str s) -> Item* {
    const auto [key, val] = parse_kv(s);
    if (auto item = this->find_item(key)) {
      item->push_val(val);
      return val.is_empty() ? item : nullptr;
    }
    return nullptr;
  }

  auto parse_opts(Slice<const Str> vals) -> Vec<Str> {
    auto pos_args = Vec<Str>{};
    auto end_of_opts = false;

    auto prev_item = static_cast<Item*>(nullptr);
    for (auto s : vals) {
      if (!s) {
        continue;
      }

      if (end_of_opts) {
        pos_args.push(s);
        continue;
      }

      // --
      if (s == "--") {
        prev_item = nullptr;
        end_of_opts = true;
        continue;
      }

      // -k, --key, --key=val
      if (s[0] == '-') {
        prev_item = this->parse_opt(s);
        continue;
      }

      // val
      if (prev_item && !prev_item->is_complete()) {
        prev_item->push_val(s);
        continue;
      }

      pos_args.push(s);
    }

    return pos_args;
  }

  auto parse_args(Slice<const Str> vals) -> Slice<const Str> {
    for (auto& item : _items) {
      if (item._type != Item::Type::Arg || item.value()) {
        continue;
      }
      if (vals.is_empty()) {
        break;
      }
      vals = item.push_vals(vals);
    }
    return vals;
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
      const auto s = item.value().unwrap_or({});
      const auto b = yes_or_no(s);
      return b;
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

auto Clap::parse(Slice<const Str> args) -> bool {
  auto parser = Parser{_items.as_mut_slice()};
  if (!parser.parse(args)) {
    return false;
  }
  const auto unset_args_cnt = parser.unuset_args_cnt();
  return unset_args_cnt == 0;
}

auto Clap::parse_cmdline(int argc, const char* argv[]) -> bool {
  auto args = Vec<Str>::with_capacity(static_cast<usize>(argc - 1));
  for (auto i = 1; i < argc; i++) {
    args.push(Str{argv[i]});
  }
  return this->parse(args.as_slice());
}

}  // namespace sfc::app
