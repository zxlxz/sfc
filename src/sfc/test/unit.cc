#include "sfc/test/unit.h"

#include "sfc/io.h"

namespace sfc::test {

auto Unit::suite() const -> Str {
  const auto p = _str.rfind(':');
  return p ? _str[{0, *p - 1}] : Str{};
}

auto Unit::name() const -> Str {
  const auto pos = _str.rfind(':');
  const auto res = pos ? _str[{*pos + 1, _}] : _str;
  return res;
}

auto Unit::invoke(bool color) const -> bool {
  const auto timer = time::Instant::now();
  auto on_run = [&](const auto&... args) {
    const auto s = color ? Str{"\e[32m[ RUN      ]\e[39m {}"} : Str{"[ RUN      ] {}"};
    io::println(s, args...);
  };

  auto on_ok = [&](const auto&... args) {
    const auto s = color ? Str{"\e[32m[       OK ]\e[39m {} ({})"} : Str{"[       OK ] {} ({})"};
    io::println(s, args..., timer.elapsed());
  };

  auto on_failed = [&](const auto&... args) {
    const auto s = color ? Str{"\e[31m[  FAILED  ]\e[39m {} ({})"} : Str{"[  FAILED  ] {} ({})"};
    io::println(s, args..., timer.elapsed());
  };

  on_run(_str);
  try {
    (_fun)();
    on_ok(_str);
  } catch (...) {
    on_failed(_str);
    return false;
  }

  return true;
}

auto Unit::match(Slice<const Str> pats) const -> bool {
  if (pats.is_empty()) {
    return true;
  }

  const auto suite = this->suite();
  const auto name = this->name();

  for (auto pat : pats) {
    const auto [pat_suite, pat_xname] = pat.split_at(suite.len());

    if (!suite.starts_with(pat_suite)) {
      continue;
    }

    if (pat_xname.is_empty() || pat_xname[{1, _}] == name) {
      return true;
    }
  }

  return false;
}

auto Suite::match(Slice<const Str> pats) const -> bool {
  if (!pats) {
    return true;
  }

  for (auto s : pats) {
    if (!s.ends_with("::")) {
      continue;
    }
    const auto p = s[{0, s.len() - 2}];
    if (_name.starts_with(p)) {
      return true;
    }
  }
  return false;
}

void Suite::invoke(Slice<const Str> pats, bool color) {
  if (!this->match(pats)) {
    return;
  }

  for (auto& unit : _unit.as_mut_slice()) {
    if (!unit.match(pats)) {
      continue;
    }
    unit.invoke(color);
  }
}

auto Tester::instance() -> Tester& {
  static auto res = Tester{};
  return res;
}

auto Tester::suites() const -> Slice<const Suite> {
  return _suites.as_slice();
}

auto Tester::operator[](Str name) -> Suite& {
  auto pos = _suites.iter_mut().find([&](auto& x) { return x.name() == name; });
  if (pos) {
    return *pos;
  }

  _suites.push(Suite{name});
  return _suites[_suites.len() - 1];
}

void Tester::invoke(Slice<const Str> pats, bool color) {
  for (auto& suite : _suites.as_mut_slice()) {
    suite.invoke(pats, color);
  }
}

}  // namespace sfc::test
