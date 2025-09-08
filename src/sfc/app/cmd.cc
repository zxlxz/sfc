#include "sfc/app/cmd.h"
#include "sfc/io/stdio.h"

namespace sfc::app {

struct Cmd::XArg {
  char _short;
  String _long;
  String _desc;
  String _default;
  String _value;
  bool _has_value = false;

  static auto from(const Arg& arg) -> XArg {
    return XArg{arg._short,
                String::from(arg._long),
                String::from(arg._desc),
                String::from(arg._default),
                String::from(arg._default)};
  }

  void update(Str val) {
    _has_value = true;
    _value = val.is_empty() ? _default.clone() : String::from(val);
  }

  auto get() const -> Option<Str> {
    if (!_has_value) {
      return {};
    }
    return _value.as_str();
  }

  auto match(Str s) const -> bool {
    if (s.is_empty()) {
      return false;
    }
    if (s.len() == 1) {
      return _short == s[0];
    }
    return _long == s;
  }

  auto flag() const -> Option<bool> {
    if (!_has_value) {
      return {};
    }

    if (_value == "1" || _value == "true" || _value == "yes" || _value == "on") {
      return true;
    }

    if (_value == "0" || _value == "false" || _value == "no" || _value == "off") {
      return false;
    }

    return {};
  }

  void fmt(auto& f) const {
    f.write_str("  ");
    if (_short != 0) {
      f.write_fmt("-{}, ", _short);
    }
    f.write_fmt("--{}", _long);
    f.write_fmt("\n          {}", _desc);
    if (!_default.is_empty()) {
      f.write_fmt(" [default: {}]", _default);
    }
  }
};

Cmd::Cmd(Str name) noexcept : _name{String::from(name)} {}

Cmd::~Cmd() noexcept {}

auto Cmd::author(Str s) -> Cmd& {
  _author = String::from(s);
  return *this;
}

auto Cmd::version(Str s) -> Cmd& {
  _version = String::from(s);
  return *this;
}

auto Cmd::about(Str s) -> Cmd& {
  _about = String::from(s);
  return *this;
}

auto Cmd::arg(Arg arg) -> Cmd& {
  _args.push(XArg::from(arg));
  return *this;
}

auto Cmd::get(Str s) const -> Option<Str> {
  if (!s) {
    return {};
  }

  for (auto& arg : _args) {
    if (arg.match(s)) {
      return arg.get();
    }
  }
  return {};
}

auto Cmd::get_flag(Str s) const -> Option<bool> {
  if (!s) {
    return {};
  }

  for (auto& arg : _args) {
    if (arg.match(s)) {
      return arg.flag();
    }
  }
  return {};
}

auto Cmd::update(Str key, Str val) -> bool {
  if (!key) {
    return false;
  }

  for (auto& arg : _args) {
    if (arg.match(key)) {
      arg.update(val);
      return true;
    }
  }

  return false;
}

void Cmd::parse(Slice<const Str> args) {
  auto last_key = Str{};
  for (auto arg : args) {
    if (!arg) {
      continue;
    }

    if (arg[0] == '-') {
      auto str = arg.trim_matches('-');
      auto pos = str.find('=').unwrap_or(str.len());
      auto key = str[{0, pos}];
      auto val = str[{pos + 1, _}];
      this->update(key, val);
      break;
    }

    if (!last_key.is_empty()) {
      const auto key = last_key;
      const auto val = arg;
      this->update(key, val);
      last_key = {};
      break;
    }
  }
}

void Cmd::parse_cmdline(int argc, const char* argv[]) {
  Str args[64] = {};
  for (auto i = 1; i < argc; i++) {
    args[i] = Str::from_cstr(argv[i]);
  }
  this->parse({args, static_cast<usize>(argc - 1)});
}

void Cmd::print_help() const {
  for (const auto& arg : _args) {
    io::println("{}", arg);
  }
}

}  // namespace sfc::app
