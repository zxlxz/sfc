#include "sfc/test.h"

#include "sfc/io.h"

namespace sfc::test {

static auto parse_filter(Str s) -> Tuple<Str, Str> {
  if (s.is_empty()) {
    return Tuple{Str{}, Str{}};
  }
  if (auto p = s.find('.')) {
    const auto i = *p;
    return Tuple{s[{0, i}], s[{i + 1, $}]};
  }
  if (auto p = s.rfind(':')) {
    const auto i = *p;
    if (i != 0 && s[i - 1] == ':') {
      return Tuple{s[{0, i - 1}], s[{i + 1, $}]};
    }
  }
  return {s, Str{}};
};

auto Case::suite() const -> Str {
  const auto p = _str.rfind(':');
  return p ? _str[{0, *p - 1}] : Str{};
}

auto Case::name() const -> Str {
  const auto pos = _str.rfind(':');
  const auto res = pos ? _str[{*pos + 1, $}] : _str;
  return res;
}

auto Case::run(bool color) const -> bool {
  const auto s_run = color ? Str{"\033[32m[ RUN      ]\033[39m"} : Str{"[ RUN      ]"};
  const auto s_suc = color ? Str{"\033[32m[       OK ]\033[39m"} : Str{"[       OK ]"};
  const auto s_err = color ? Str{"\033[31m[  FAILED  ]\033[39m"} : Str{"[  FAILED  ]"};

  const auto suite_name = this->suite();
  const auto test_name = this->name();
  const auto timer = time::Instant::now();

  io::println("{} {}.{}", s_run, suite_name, test_name);
  try {
    (_fun)();
    io::println("{} {}.{} ({} ms)", s_suc, suite_name, test_name, timer.elapsed().as_millis());
  } catch (...) {
    io::println("{} {}.{} ({} ms)", s_err, suite_name, test_name, timer.elapsed().as_millis());
    return false;
  }

  return true;
}

Suite::Suite(Str name) : _name{String::from(name)}, _cases{_cases.with_capacity(64)} {}

Suite::~Suite() {
  _cases.clear();
}

auto Suite::name() const -> Str {
  return _name;
}

auto Suite::tests() const -> Slice<const Case> {
  return _cases.as_slice();
}

auto Suite::push(Case unit) -> Case& {
  return _cases.push(unit);
}

void Suite::run(Slice<const Str> pats, bool color) {
  if (!pats) {
    for (auto& test : _cases.as_mut_slice()) {
      test.run(color);
    }
    return;
  }

  for (auto pat : pats) {
    const auto [suite_name, test_name] = parse_filter(pat);
    if (suite_name != _name) {
      continue;
    }

    for (auto& test : _cases.as_mut_slice()) {
      if (test_name == test.name()) {
        test.run(color);
      }
    }
  }
}

auto AllTest::instance() -> AllTest& {
  static auto res = AllTest{};
  return res;
}

auto AllTest::suites() -> Slice<Suite> {
  return _suites.as_mut_slice();
}

auto AllTest::regist(Case test_case) -> bool {
  auto find_suite = [this](Str name) -> Suite& {
    for (auto& suite : _suites.as_mut_slice()) {
      if (suite.name() == name) {
        return suite;
      }
    }
    return _suites.push(Suite{name});
  };

  const auto suite_name = test_case.suite();
  auto& suite = find_suite(suite_name);
  suite.push(test_case);
  return true;
}

void AllTest::run(Slice<const Str> pats, bool color) {
  for (auto& suite : _suites.as_mut_slice()) {
    suite.run(pats, color);
  }
}

}  // namespace sfc::test
