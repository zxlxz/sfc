#include "sfc/app/opts.h"

namespace sfc::app {

static auto parse_opt(Str s) -> Tuple<Str, Str> {
  if (s[0] != '-') {
    return {s, {}};
  }

  s = s[1] == '-' ? s[{2, _}] : s[{1, _}];
  const auto p = s.find('=');
  if (!p) {
    return {s, {}};
  }
  return {s[{0, *p}], s[{*p + 1, _}]};
}

Opts::Opts() = default;

Opts::~Opts() = default;

void Opts::add_opt(Str name, Str desc, char short_name, Str default_value) {
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

auto Opts::get(Str s) const -> Str {
  const auto c = s.len() == 1 ? s[0] : 0;

  for (const auto& x : _items) {
    if ((c && c == x.short_name) || (!c && s == x.name)) {
      const auto set_val = x.value.as_str();
      const auto default_val = x.default_value.as_str();
      return set_val ? set_val : default_val;
    }
  }
  return {};
}

auto Opts::has_flag(Str s) const -> bool {
  const auto c = s.len() == 1 ? s[0] : 0;

  for (const auto& x : _items) {
    if ((c && c == x.short_name) || (!c && s == x.name)) {
      const auto set_val = x.value.as_str();
      const auto default_val = x.default_value.as_str();
      const auto t = set_val ? set_val : default_val;
      return t == "1" || t == "on" || t == "yes" || t == "true";
    }
  }
  return false;
}

auto Opts::usage() const -> String {
  auto res = String{};
  auto out = fmt::Fmter{res};
  for (const auto& x : _items) {
    out.write_str("  ");
    if (x.short_name != 0) {
      out.write_fmt("-{}, ", x.short_name);
    }
    out.write_fmt("--{}", x.name);
    out.write_str("\n          ");
    out.write_str(x.desc);
    if (!x.default_value.is_empty()) {
      out.write_fmt(" [default: {}]", x.default_value);
    }
    out.write_str("\n");
  }
  return res;
}

void Opts::parse(Slice<const Str> args) {
  auto last_key = Str{};

  for (auto arg : args) {
    if (arg.starts_with("-")) {
      const auto [key, val] = parse_opt(arg);
      last_key = key;
      this->update(key, val);
      continue;
    }

    if (!last_key.is_empty()) {
      this->update(last_key, arg);
      last_key = {};
      continue;
    }
  }
}

auto Opts::update(Str s, Str value) -> bool {
  const auto c = s.len() == 1 ? s[0] : 0;

  for (auto& x : _items) {
    if ((c && c == x.short_name) || (!c && s == x.name)) {
      const auto val = value ? value : x.default_value.as_str();
      x.value = String::from(val ? val : "1");
      return true;
    }
  }

  return false;
}

}  // namespace sfc::app
