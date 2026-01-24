#include "sfc/test/test.h"

#include "sfc/io.h"
#include "sfc/time.h"

namespace sfc::test {

static auto unpack_type(Str type) -> Tuple<Str, Str> {
  const auto pos = type.rfind(':').unwrap_or(0);
  if (pos > 1) {
    return {type[{0, pos - 1}], type[{pos + 1, $}]};
  }
  return {Str{}, type};
}

static auto unpack_pat(Str pat) -> Tuple<Str, Str> {
  if (auto p = pat.find('.')) {
    return {pat[{0, *p}], pat[{*p + 1, $}]};
  }

  return {Str{}, pat};
}

auto Test::suite() const -> Str {
  return unpack_type(_type).get<0>();
}

auto Test::name() const -> Str {
  return unpack_type(_type).get<1>();
}

auto Test::match(Str pat) const -> bool {
  if (pat.is_empty() || pat == "*") {
    return true;
  }

  if (_type.len() == pat.len()) {
    return _type == pat;
  }

  if (_type.len() > pat.len()) {
    const auto colon_pos = _type.len() - pat.len() - 1;
    return _type[colon_pos] == ':' && _type.ends_with(pat);
  }

  return false;
}

auto Test::run(bool color) const -> bool {
  const auto s_run = color ? Str{"\033[32m[ RUN      ]\033[39m"} : Str{"[ RUN      ]"};
  const auto s_suc = color ? Str{"\033[32m[       OK ]\033[39m"} : Str{"[       OK ]"};
  const auto s_err = color ? Str{"\033[31m[  FAILED  ]\033[39m"} : Str{"[  FAILED  ]"};

  const auto [suite_name, test_name] = unpack_type(_type);
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

void Suite::push(Test test) {
  if (_tests.is_empty()) {
    _tests.reserve(32);
  }
  _tests.push(test);
}

auto Suite::name() const -> Str {
  return _name;
}

auto Suite::tests() const -> Slice<const Test> {
  return _tests.as_slice();
}

auto Suite::match(Str pat) const -> bool {
  return pat.is_empty() || pat == "*" || pat == _name;
}

void Suite::run(bool color) const {
  const auto title = color ? Str{"\033[32m[----------]\033[39m"} : Str{"[----------]"};

  io::println("{} {} tests from {}", title, _tests.len(), _name);
  const auto timer = time::Instant::now();
  for (auto& test : _tests.as_slice()) {
    test.run(color);
  }
  const auto dur_ms = timer.elapsed().as_millis();
  io::println("{} {} tests from {} ({} ms total)", title, _tests.len(), _name, dur_ms);
}

auto Suite::filter(Slice<const Str> pats) const -> Suite {
  auto res = Suite{
      ._name = _name,
  };

  for (auto& test : _tests.as_slice()) {
    if (pats.is_empty()) {
      res.push(test);
    }
    for (auto pat : pats) {
      const auto [suite_pat, test_pat] = unpack_pat(pat);
      if (this->match(suite_pat) && test.match(test_pat)) {
        res.push(test);
        break;
      }
    }
  }

  return res;
}

static auto _all_suites() -> Vec<Suite>& {
  static auto s_suites = Vec<Suite>{};
  return s_suites;
}

auto suites() -> Slice<const Suite> {
  return _all_suites().as_slice();
}

auto get_suite(Str suite_name) -> Suite& {
  auto& suites = _all_suites();

  for (auto& suite : suites.as_mut_slice()) {
    if (suite._name == suite_name) {
      return suite;
    }
  }

  suites.push(Suite{._name = suite_name});
  return suites.last_mut().unwrap();
}

auto regist(Test test) -> bool {
  auto& suite = get_suite(test.suite());

  for (auto& t : suite._tests.as_slice()) {
    if (t._type == test._type) {
      return false;
    }
  }
  suite.push(test);
  return true;
}

void run(Slice<const Suite> suites, bool color) {
  const auto title = color ? Str{"\033[32m[==========]\033[39m"} : Str{"[==========]"};

  auto tests_cnt = usize{0U};
  for (auto& suite : suites) {
    tests_cnt += suite.tests().len();
  }

  io::println("Running {} tests from {} test suites.", title, tests_cnt, suites.len());
  const auto timer = time::Instant::now();
  for (auto& suite : suites) {
    suite.run(color);
    io::println("");
  }
  const auto dur_ms = timer.elapsed().as_millis();
  io::println("{} {} tests ran. ({} ms total)", title, tests_cnt, dur_ms);
}

auto filter(Slice<const Str> pats) -> Vec<Suite> {
  auto res = Vec<Suite>{};

  for (auto& suite : _all_suites().as_slice()) {
    auto filtered_suite = suite.filter(pats);
    if (!filtered_suite._tests.is_empty()) {
      res.push(mem::move(filtered_suite));
    }
  }
  return res;
}

}  // namespace sfc::test
