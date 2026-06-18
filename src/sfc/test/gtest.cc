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

struct GTestCase {
  Test _test;
  GTestResult _result = GTestResult::None;

 public:
  auto name() const -> Str {
    return _test.name();
  }

  auto match(Str full_pat) const -> bool {
    if (full_pat.is_empty() || full_pat == "*") {
      return true;
    }

    const auto name = _test.name();
    const auto p = full_pat.rfind('.');
    if (!p) {
      return name == full_pat;
    }

    const auto pat = full_pat[{*p + 1, $}];
    if (pat.is_empty() || pat == "*") {
      return true;
    }

    if (pat == name) {
      return true;
    }

    return false;
  }

  auto match_pats(Slice<const Str> pats) const -> bool {
    return pats.is_empty() || pats.iter().any([&](Str pat) { return this->match(pat); });
  }

  auto run(bool use_color) -> bool {
    _result = GTestResult::None;
    const auto s_run = use_color ? Str{"\033[32m[ RUN      ]\033[39m"} : Str{"[ RUN      ]"};
    const auto s_suc = use_color ? Str{"\033[32m[       OK ]\033[39m"} : Str{"[       OK ]"};
    const auto s_err = use_color ? Str{"\033[31m[  FAILED  ]\033[39m"} : Str{"[  FAILED  ]"};

    const auto timer = time::Instant::now();
    io::println("{} {}", s_run, *this);
    try {
      _test.run();
      io::println("{} {} ({} ms)", s_suc, *this, timer.elapsed().as_millis());
      _result = GTestResult::Passed;
    } catch (...) {
      io::println("{} {} ({} ms)", s_err, *this, timer.elapsed().as_millis());
      _result = GTestResult::Failed;
      return false;
    }

    return true;
  }

  void fmt(auto& f) const {
    f.write_fmt("{}.{}", _test.mod(), _test.name());
  }
};

struct GTestSuite {
  const Module& _mod;  // Module is global and never modified, so reference is safe
  List<GTestCase> _cases{};

 public:
  auto match(Str pats) const -> bool {
    const auto pats_len = pats.len();

    if (pats_len == 0) {
      return true;
    }

    const auto name = _mod.name();
    const auto name_len = name.len();

    // A::
    if (pats_len < name_len) {
      return pats.ends_with(':') && name.starts_with(pats);
    }

    // A::B
    if (pats_len == name_len) {
      return name == pats;
    }

    // A::B.C
    if (pats_len > name_len) {
      return pats[name_len] == '.' && pats.starts_with(name);
    }
    return false;
  }

  auto match_pats(Slice<const Str> pats) const -> bool {
    return pats.is_empty() || pats.iter().any([&](Str pat) { return this->match(pat); });
  }

  auto cases() const noexcept -> Slice<const GTestCase> {
    return _cases.as_slice();
  }

  void filter(Slice<const Str> pats) {
    _cases.clear();
    for (auto& test : _mod.tests()) {
      auto test_case = GTestCase{test};
      if (!this->match_pats(pats)) continue;
      if (!test_case.match_pats(pats)) continue;
      _cases.push(test_case);
    }
  }

  auto passed_cnt() const -> usize {
    auto cnt = usize{0U};
    for (const auto& test_case : _cases.as_slice()) {
      if (test_case._result == GTestResult::Passed) {
        cnt += 1;
      }
    }
    return cnt;
  }

  auto failed_cnt() const -> usize {
    auto cnt = usize{0U};
    for (const auto& test_case : _cases.as_slice()) {
      if (test_case._result == GTestResult::Failed) {
        cnt += 1;
      }
    }
    return cnt;
  }

  void run(bool use_color) {
    const auto suite_name = _mod.name();
    const auto cases_cnt = _cases.len();
    const auto title = use_color ? Str{"\033[32m[----------]\033[39m"} : Str{"[----------]"};

    io::println("{} {} tests from {}", title, cases_cnt, suite_name);
    const auto timer = time::Instant::now();
    for (auto& test_case : _cases.as_mut_slice()) {
      test_case.run(use_color);
    }
    const auto dur_ms = timer.elapsed().as_millis();
    io::println("{} {} tests from {} ({} ms total)", title, cases_cnt, suite_name, dur_ms);
    io::println("");
  }

  void show_fails(Str title) const {
    for (const auto& test_case : _cases.as_slice()) {
      if (test_case._result == GTestResult::Failed) {
        io::println("{} {}", title, test_case);
      }
    }
  }
};

struct GTestRun {
  List<GTestSuite> _suites{};

 public:
  void collect_suites(Str filter) {
    auto pats = this->parse_filter(filter);
    _suites.clear();

    for (auto& mod : test::modules()) {
      auto suite = GTestSuite{mod};
      suite.filter(pats.as_slice());
      if (suite.cases().is_empty()) {
        continue;
      }
      _suites.push(mem::move(suite));
    }
  }

  auto suite_cnt() const -> usize {
    return _suites.len();
  }

  auto tests_cnt() const -> usize {
    auto cnt = usize{0U};
    for (const auto& suite : _suites.as_slice()) {
      cnt += suite.cases().len();
    }
    return cnt;
  }

  auto passed_cnt() const -> usize {
    auto cnt = usize{0U};
    for (const auto& suite : _suites.as_slice()) {
      cnt += suite.passed_cnt();
    }
    return cnt;
  }

  auto failed_cnt() const -> usize {
    auto cnt = usize{0U};
    for (const auto& suite : _suites.as_slice()) {
      cnt += suite.failed_cnt();
    }
    return cnt;
  }

  void run(bool color) {
    const auto suite_cnt = this->suite_cnt();
    const auto tests_cnt = this->tests_cnt();

    const auto title = color ? Str{"\033[32m[==========]\033[39m"} : Str{"[==========]"};
    io::println("{} Running {} tests from {} test suites.", title, tests_cnt, suite_cnt);

    const auto timer = time::Instant::now();
    for (auto& suite : _suites.as_mut_slice()) {
      suite.run(color);
    }
    const auto dur_ms = timer.elapsed().as_millis();
    io::println("{} {} tests from {} test suites ran. ({} ms total)", title, tests_cnt, suite_cnt, dur_ms);
    this->show_summary(color);
  }

 private:
  auto parse_filter(Str filter) -> List<Str> {
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

  void show_summary(bool color) {
    const auto passed_title = color ? Str{"\033[32m[  PASSED  ]\033[39m"} : Str{"[  PASSED  ]"};
    const auto failed_title = color ? Str{"\033[31m[  FAILED  ]\033[39m"} : Str{"[  FAILED  ]"};

    const auto passed_cnt = this->passed_cnt();
    io::println("{} {} tests.", passed_title, passed_cnt);

    const auto failed_cnt = this->failed_cnt();
    if (failed_cnt == 0) {
      return;
    }
    io::println("{} {} tests. listed below:", failed_title, failed_cnt);
    for (const auto& suite : _suites.as_slice()) {
      suite.show_fails(failed_title);
    }

    io::println("");
    io::println("{} FAILED TESTS", failed_cnt);
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

void gtest_run(Str filter, bool color) {
  auto runner = GTestRun{};
  runner.collect_suites(filter);
  runner.run(color);
}

}  // namespace sfc::test
