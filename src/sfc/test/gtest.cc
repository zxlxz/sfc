#include "sfc/test/gtest.h"
#include "sfc/test/test.h"
#include "sfc/io/stdio.h"
#include "sfc/time.h"
#include "sfc/fs.h"

namespace sfc::test {

enum class GTestResult {
  None,
  Passed,
  Failed,
};

class GTestCase {
  Test _test;
  Str _suite_name;
  Str _test_name;
  GTestResult _result{};
  usize _duration_ms{};

 public:
  GTestCase(Test test) : _test{test} {
    _suite_name = _test.mod();
    _test_name = _test.name();
  }

  auto name() const -> Str {
    return _test_name;
  }

  auto result() const -> GTestResult {
    return _result;
  }

  auto match(Slice<const Str> pats) const -> bool {
    if (pats.is_empty()) {
      return true;
    }
    return pats.iter().any([&](Str pat) { return this->match_imp(pat); });
  }

  void run(bool use_color) {
    _result = GTestResult::None;
    this->show_result(use_color);
    const auto timer = time::Instant::now();
    try {
      _test.run();
      _result = GTestResult::Passed;
    } catch (...) {
      _result = GTestResult::Failed;
    }
    _duration_ms = timer.elapsed().as_millis();
    this->show_result(use_color);
  }

 public:
  void fmt(auto& f) const {
    if (_suite_name.is_empty()) {
      f.write_fmt("{}", _test_name);
    } else {
      f.write_fmt("{}.{}", _suite_name, _test_name);
    }
  }

 private:
  auto match_imp(Str pat) const -> bool {
    if (pat.is_empty() || pat == "*") {
      return true;
    }

    const auto pat_pos = pat.rfind('.');
    const auto pat_name = pat_pos ? pat[{*pat_pos + 1, $}] : pat;
    if (pat_name.is_empty() || pat_name == "*") {
      return true;
    }

    if (pat_name == _test_name) {
      return true;
    }

    return false;
  }

  void show_result(bool use_color) {
    switch (_result) {
      case GTestResult::None: {
        const auto s_run = !use_color ? Str{"[ RUN      ]"} : Str{"\033[32m[ RUN      ]\033[39m"};
        io::println("{} {}", s_run, *this);
        break;
      }
      case GTestResult::Passed: {
        const auto s_suc = !use_color ? Str{"[       OK ]"} : Str{"\033[32m[       OK ]\033[39m"};
        io::println("{} {} ({} ms)", s_suc, *this, _duration_ms);
        break;
      }
      case GTestResult::Failed: {
        const auto s_err = !use_color ? Str{"[  FAILED  ]"} : Str{"\033[31m[  FAILED  ]\033[39m"};
        io::println("{} {} ({} ms)", s_err, *this, _duration_ms);
        break;
      }
    }
  }
};

class GTestSuite {
  const Module& _mod;  // Module is global and never modified, so reference is safe
  List<GTestCase> _cases{};
  usize _run_cnt{0};
  usize _passed_cnt{0};
  usize _failed_cnt{0};
  usize _duration_ms{};

 public:
  GTestSuite(const Module& mod) : _mod{mod} {}

  void filter(Slice<const Str> pats) {
    _cases.clear();

    if (!this->match(pats)) {
      return;
    }

    for (auto& test : _mod.tests()) {
      auto test_case = GTestCase{test};
      if (!test_case.match(pats)) continue;
      _cases.push(test_case);
    }
  }

  auto match(Slice<const Str> pats) const -> bool {
    if (pats.is_empty()) {
      return true;
    }
    return pats.iter().any([&](Str pat) { return this->match_imp(pat); });
  }

  auto cases() const noexcept -> Slice<const GTestCase> {
    return _cases.as_slice();
  }

  auto run_cnt() const -> usize {
    return _run_cnt;
  }

  auto passed_cnt() const -> usize {
    return _passed_cnt;
  }

  auto failed_cnt() const -> usize {
    return _failed_cnt;
  }

  void run(bool use_color) {
    _run_cnt = 0U;
    _passed_cnt = 0U;
    _failed_cnt = 0U;

    this->show_run(use_color);
    const auto timer = time::Instant::now();
    for (auto& test_case : _cases.as_mut_slice()) {
      _run_cnt++;
      test_case.run(use_color);
      switch (test_case.result()) {
        case GTestResult::Passed: _passed_cnt++; break;
        case GTestResult::Failed: _failed_cnt++; break;
        default:                  break;
      }
    }
    _duration_ms = timer.elapsed().as_millis();
    this->show_result(use_color);
  }

  void show_fails(bool use_color) const {
    const auto title = !use_color ? Str{"[  FAILED  ]"} : Str{"\033[31m[  FAILED  ]\033[39m"};
    for (const auto& test_case : _cases.as_slice()) {
      if (test_case.result() == GTestResult::Failed) {
        io::println("{} {}", title, test_case);
      }
    }
  }

 private:
  auto match_imp(Str pat) const -> bool {
    const auto pat_len = pat.len();

    if (pat_len == 0) {
      return true;
    }

    const auto suite_name = _mod.name();

    // A::
    if (pat_len < suite_name.len()) {
      return pat.ends_with(':') && suite_name.starts_with(pat);
    }

    // A::B
    if (pat_len == suite_name.len()) {
      return suite_name == pat;
    }

    // A::B.C
    if (pat_len > suite_name.len()) {
      return pat[suite_name.len()] == '.' && pat.starts_with(suite_name);
    }
    return false;
  }

  void show_run(bool use_color) const {
    const auto title = !use_color ? Str{"[----------]"} : Str{"\033[32m[----------]\033[39m"};
    io::println("{} {} tests from {}", title, _cases.len(), _mod.name());
  }

  void show_result(bool use_color) const {
    const auto title = !use_color ? Str{"[----------]"} : Str{"\033[32m[----------]\033[39m"};
    io::println("{} {} tests from {} ({} ms total)", title, _cases.len(), _mod.name(), _duration_ms);
    io::println("");
  }
};

class GTestRunner {
  List<GTestSuite> _suites{};
  usize _case_cnt{};

  usize _run_cnt{};
  usize _passed_cnt{};
  usize _failed_cnt{};
  usize _duration_ms{};

 public:
  void collect_suites(Slice<const Str> pats) {
    _suites.clear();
    _case_cnt = 0;

    for (auto& mod : test::modules()) {
      auto suite = GTestSuite{mod};
      suite.filter(pats);

      const auto cases = suite.cases();
      if (cases.is_empty()) {
        continue;
      }
      _case_cnt += cases.len();
      _suites.push(mem::move(suite));
    }
  }

  void run(bool use_color) {
    _run_cnt = {};
    _passed_cnt = {};
    _failed_cnt = {};
    _duration_ms = {};

    this->show_run(use_color);
    const auto timer = time::Instant::now();
    for (auto& suite : _suites.as_mut_slice()) {
      suite.run(use_color);
      _run_cnt += suite.run_cnt();
      _passed_cnt += suite.passed_cnt();
      _failed_cnt += suite.failed_cnt();
    }
    _duration_ms = timer.elapsed().as_millis();
    this->show_result(use_color);
    this->show_summary(use_color);
  }

 private:
  void show_run(bool use_color) const {
    const auto title = !use_color ? Str{"[==========]"} : Str{"\033[32m[==========]\033[39m"};
    io::println("{} Running {} tests from {} test suites.", title, _case_cnt, _suites.len());
  }

  void show_result(bool use_color) const {
    const auto title = !use_color ? Str{"[==========]"} : Str{"\033[32m[==========]\033[39m"};
    io::println("{} {} tests from {} test suites ran. ({} ms total)", title, _case_cnt, _suites.len(), _duration_ms);
  }

  void show_summary(bool use_color) {
    const auto pass_title = !use_color ? Str{"[  PASSED  ]"} : Str{"\033[32m[  PASSED  ]\033[39m"};
    const auto fail_title = !use_color ? Str{"[  FAILED  ]"} : Str{"\033[31m[  FAILED  ]\033[39m"};

    io::println("{} {} tests.", pass_title, _passed_cnt);

    if (_failed_cnt != 0) {
      io::println("{} {} tests. listed below:", fail_title, _failed_cnt);
      for (const auto& suite : _suites.as_slice()) {
        suite.show_fails(use_color);
      }
      io::println("");
      io::println("{} FAILED TESTS", _failed_cnt);
    }
  }
};

void gtest_help() {
  static constexpr char msg[] =
      "This program contains tests written using SFC Test. You can use the\n"
      "following command line flags to control its behavior: \n"
      "\n"
      "Test Selection:\n"
      "  --gtest_list_tests\n"
      "      List the names of all tests instead of running them.\n"
      "  --gtest_filter=POSITIVE_PATTERNS\n"
      "      Run only the tests whose name starts with the pattern.";
  io::println(msg);
}

auto gtest_all_count() -> usize {
  auto cnt = 0UZ;
  for (const auto& mod : test::modules()) {
    cnt += mod.count();
  }
  return cnt;
}

auto gtest_format_xml() -> String {
  const auto all_cnt = test::gtest_all_count();
  const auto all_suites = test::modules();

  auto buf = String{};
  auto out = fmt::Formatter{buf};
  out.write_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  out.write_fmt("<testsuites tests=\"{}\" name=\"AllTests\">\n", all_cnt);
  for (auto& suite : all_suites) {
    auto tests = suite.tests();
    out.write_fmt("<testsuite name=\"{}\" tests=\"{}\">\n", suite.name(), tests.len());
    for (auto& test : tests) {
      const auto name = test.name();
      const auto file = Str::from_cstr(test._loc.file);
      const auto line = test._loc.line;
      out.write_fmt("<testcase name=\"{}\" file=\"{}\" line=\"{}\"/>\n", name, file, line);
    }
    out.write_str("  </testsuite>\n");
  }
  out.write_str("</testsuites>\n");
  return buf;
}

void gtest_list_tests_xml(Str path) {
  const auto xml = test::gtest_format_xml();
  if (path.is_empty() || path == "-" || path == "stdout") {
    io::println("{}", xml);
    return;
  }

  auto file = fs::File::create(path).expect("create output file failed");
  (void)file.write_str(xml.as_str());
}

void gtest_list_tests(Str uri) {
  auto path = uri;
  if (auto type = Str("xml:"); uri.starts_with(type)) {
    path = uri[{type.len(), $}];
  }

  test::gtest_list_tests_xml(path);
}

auto gtest_split_pats(Str filter) -> List<Str> {
  auto find_pos = [](Str s) -> usize {
    for (auto p = 0U; p < s._len; p++) {
      switch (s[p]) {
        case ';': return p;
        case ':':
          if (s[p + 1] != ':') return p;  // pass '::'
          p += 1;
      }
    }
    return s._len;
  };

  auto pats = List<Str>{};
  while (!filter.is_empty()) {
    const auto pos = find_pos(filter);
    pats.push(filter[{0, pos}]);
    filter = filter[{pos + 1, $}];
  }

  return pats;
}

void gtest_run(Str filter, bool color) {
  const auto pats = gtest_split_pats(filter);

  auto runner = GTestRunner{};
  runner.collect_suites(pats.as_slice());
  runner.run(color);
}

}  // namespace sfc::test
