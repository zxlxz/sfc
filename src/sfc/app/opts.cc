#include "sfc/app/opts.h"

namespace sfc::app {

struct Opts::Inn {
  struct Item {
    char short_name{0};
    String name{};
    String desc{};
    String default_value{};

    bool has_value{};
    String value{};
  };

  vec::Vec<Item> _items;
  Str _last_key{};

 public:
  void add_opt(char short_name, Str name, Str desc, Str default_value) {
    if (name.starts_with("--")) {
      name = name[{2, _}];
    } else if (name.starts_with("-")) {
      name = name[{1, _}];
    }

    auto item = Item{
        .short_name = short_name,
        .name = String::from(name),
        .desc = String::from(desc),
        .default_value = String::from(default_value),
    };
    _items.push(mem::move(item));
  }

  auto get(Str name) const -> Option<const Item&> {
    if (!name) {
      return {};
    }

    const auto short_name = name.len() == 1 ? name[0] : 0;

    for (auto& item : _items) {
      if (short_name && short_name == item.short_name) {
        return item;
      }
      if (name == item.name) {
        return item;
      }
    }
    return {};
  }

  void update(Str name, Str value) {
    auto opt = this->get(name);
    if (!opt) {
      return;
    }

    auto& val = const_cast<Item&>(*opt);
    val.has_value = true;
    val.value = String::from(value);
  }

  auto usage() const -> String {
    auto res = String{};
    for (const auto& item : _items) {
      res.push_str("  ");
      if (item.short_name != 0) {
        res.push('-');
        res.push(item.short_name);
        res.push_str(", ");
      }
      res.push_str("--");
      res.push_str(item.name);
      res.push_str("\n          ");
      res.push_str(item.desc);
      if (!item.default_value.is_empty()) {
        res.push_str(" [default: ");
        res.push_str(item.default_value);
        res.push_str("]");
      }
      res.push_str("\n");
    }
    return res;
  }

  auto parse_arg(Str arg) -> bool {
    if (arg[0] == '-') {
      const auto s = arg[{arg[1] == '-' ? 2U : 1U, _}];
      const auto p = s.find('=');
      const auto k = p ? s[{0, *p}] : s;
      const auto v = p ? s[{*p + 1, _}] : Str{};
      this->update(k, v);
      _last_key = p ? Str{} : k;
      return true;
    }

    if (!_last_key.is_empty()) {
      this->update(_last_key, arg);
      _last_key = {};
      return true;
    }

    return false;
  }
};

Opts::Opts() : _inn{Box<Inn>::xnew()} {}

Opts::~Opts() {}

void Opts::add_opt(Str name, Str desc, char short_name, Str default_value) {
  _inn->add_opt(short_name, name, desc, default_value);
}

auto Opts::has(Str s) const -> bool {
  const auto opt = _inn->get(s);
  if (!opt) {
    return false;
  }

  return opt->has_value;
}

auto Opts::get(Str s) const -> Option<Str> {
  const auto opt = _inn->get(s);
  if (!opt) {
    return {};
  }

  auto& item = *opt;
  const auto& val = item.has_value ? item.value : item.default_value;
  return val.as_str();
}

auto Opts::get_flag(Str name) const -> Option<bool> {
  const auto opt = this->get(name);
  if (!opt) {
    return {};
  }

  const auto val = *opt;
  if (val == "1" || val == "true" || val == "yes" || val == "on") {
    return true;
  }

  if (val == "0" || val == "false" || val == "no" || val == "off") {
    return false;
  }

  return {};
}

auto Opts::usage() const -> String {
  return _inn->usage();
}

void Opts::parse_args(Slice<const Str> args) {
  for (auto arg : args) {
    _inn->parse_arg(arg);
  }
}

void Opts::parse_cmdline(int argc, const char* argv[]) {
  for (auto i = 1; i < argc; i++) {
    auto arg = Str::from(argv[i]);
    _inn->parse_arg(arg);
  }
}

}  // namespace sfc::app
