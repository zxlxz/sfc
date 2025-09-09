#include "sfc/app/cmd.h"
#include "sfc/io/stdio.h"

namespace sfc::app {

struct Cmd::XArg {
  char _short;
  String _long;
  String _help;
  String _default = {};
  Vec<String> _values = {};

 public:
  static auto from(const Arg& t) -> XArg {
    return {t._short, String::from(t._long), String::from(t._help), String::from(t._default)};
  }

  auto match(Str key) const -> bool {
    if (key.len() == 1) {
      return key[0] == _short;
    }

    if (_long.ends_with('*')) {
      return key == _long[{0, key.len() - 1}];
    }

    return key == _long;
  }

  auto update(Str key) -> XArg* {
    _values.push(String::from(key));

    if (!_long.is_empty() && !_long.ends_with("*")) {
      return nullptr;
    }
    return this;
  }

  auto var() const -> Option<Str> {
    if (_values.is_empty()) {
      return {};
    }
    return _values[0].as_str();
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

Cmd::Cmd(Str name) noexcept : _name{String::from(name)} {
  this->add_arg({});
}

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
  _args.push(XArg::from(arg));
}

void Cmd::add_flag(Arg arg) {
  _args.push(XArg::from(arg));
}

auto Cmd::get(Str s) const -> Option<Str> {
  for (auto& xarg : _args) {
    if (xarg.match(s)) {
      return xarg.var();
    }
  }
  return {};
}

auto Cmd::get_flag(Str s) const -> Option<bool> {
  for (auto& xarg : _args) {
    if (xarg.match(s)) {
      return xarg.flag();
    }
  }
  return {};
}

auto Cmd::get_args(Str s) const -> Slice<const String> {
  for (auto& xarg : _args) {
    if (xarg.match(s)) {
      return xarg._values.as_slice();
    }
  }
  return {};
}

void Cmd::parse(Slice<const Str> args) {
  auto parse_kv = [](Str s) -> Tuple<Str, Str> {
    s = s.trim_start_matches('-');
    const auto p = s.find('=');
    if (!p) {
      return {s, {}};
    }
    return {s[{0, *p}], s[{*p + 1, _}]};
  };

  auto find_xarg = [this](Str s) -> XArg* {
    for (auto& xarg : _args) {
      if (xarg.match(s)) {
        return &xarg;
      }
    }
    return nullptr;
  };

  auto prev_xarg = static_cast<XArg*>(nullptr);
  for (auto arg : args) {
    if (!arg) {
      continue;
    }

    if (arg[0] == '-') {
      const auto [k, v] = parse_kv(arg);
      if ((prev_xarg = find_xarg(k))) {
        prev_xarg->update(v);
      }
      continue;
    }

    if (prev_xarg) {
      prev_xarg = prev_xarg->update(arg);
    } else {
      _args[0].update(arg);
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
  for (const auto& arg : _args) {
    io::println("    {}", arg);
  }
}

}  // namespace sfc::app
