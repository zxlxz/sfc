#include "sfc/test/gtest.h"
#include "sfc/test/test.h"
#include "sfc/io/stdio.h"
#include "sfc/time.h"
#include "sfc/fs.h"

namespace sfc::test {

enum class Color {
  RED = 1,
  GREEN = 2,
  YELLOW = 3,
  BLUE = 4,
  MAGENTA = 5,
  CYAN = 6,
  RESET = 7,
};

struct ColoredStr {
  Str _str;
  Color _fg_color;
  bool _enable_color;

 public:
  void fmt(auto& f) const {
    if (!_enable_color) {
      f.write_str(_str);
    } else {
      f.write_str(fg(_fg_color));
      f.write_str(_str);
      f.write_str(fg(Color::RESET));
    }
  }

 private:
  auto fg(Color c) const -> Str {
    switch (c) {
      case Color::RED:     return "\033[31m";
      case Color::GREEN:   return "\033[32m";
      case Color::YELLOW:  return "\033[33m";
      case Color::BLUE:    return "\033[34m";
      case Color::MAGENTA: return "\033[35m";
      case Color::CYAN:    return "\033[36m";
      case Color::RESET:   return "\033[39m";
      default:             return "";
    }
  }

  auto bg(Color c) const -> Str {
    switch (c) {
      case Color::RED:     return "\033[41m";
      case Color::GREEN:   return "\033[42m";
      case Color::YELLOW:  return "\033[43m";
      case Color::BLUE:    return "\033[44m";
      case Color::MAGENTA: return "\033[45m";
      case Color::CYAN:    return "\033[46m";
      case Color::RESET:   return "\033[49m";
      default:             return "";
    }
  }
};

enum class GTestStat {
  None,
  Passed,
  Failed,
};

// GTestCase <-> TestFn
struct GTestCase {
  TestFn _inn;
  GTestStat _stat = {};
  time::Duration _duration = {};

 public:
  auto match(Slice<const Str> pats) const -> bool {
    if (pats.is_empty()) {
      return true;
    }
    return pats.iter().any([&](Str pat) { return this->match_imp(pat); });
  }

  auto run(bool use_color) -> bool {
    _stat = GTestStat::None;
    _duration = {};

    const auto timer = time::Instant::now();
    this->show(use_color);
    const auto passed = _inn.run();
    _stat = passed ? GTestStat::Passed : GTestStat::Failed;
    _duration = timer.elapsed();
    this->show(use_color);
    return passed;
  }

  void show(bool use_color) const {
    switch (_stat) {
      case GTestStat::None: {
        const auto label = ColoredStr{"[ RUN      ]", Color::GREEN, use_color};
        io::println("{} {}", label, *this);
        break;
      }
      case GTestStat::Passed: {
        const auto label = ColoredStr{"[       OK ]", Color::GREEN, use_color};
        io::println("{} {} ({} ms)", label, *this, _duration.as_millis());
        break;
      }
      case GTestStat::Failed: {
        const auto label = ColoredStr{"[  FAILED  ]", Color::RED, use_color};
        io::println("{} {} ({} ms)", label, *this, _duration.as_millis());
        break;
      }
    }
  }

 public:
  void fmt(auto& f) const {
    const auto mod = _inn._mod;
    const auto fun = _inn._name;
    if (mod.is_empty()) {
      f.write_fmt("{}", fun);
    } else {
      f.write_fmt("{}.{}", mod, fun);
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

    if (pat_name == _inn._name) {
      return true;
    }

    return false;
  }
};

// GTestSuite <-> TestMod
struct GTestSuite {
  const TestMod& _mod;
  List<GTestCase> _matchs{};

  usize _run_cnt{};
  usize _passed_cnt{};
  usize _failed_cnt{};
  time::Duration _duration{};

 public:
  GTestSuite(const TestMod& mod) : _mod{mod} {}

 public:
  auto match(Slice<const Str> pats) const -> bool {
    if (pats.is_empty()) {
      return true;
    }
    return pats.iter().any([&](Str pat) { return this->match_imp(pat); });
  }

  auto filt(Slice<const Str> pats) -> usize {
    _matchs.clear();

    if (!this->match(pats)) {
      return 0;
    }

    for (auto& test : _mod.tests()) {
      auto test_case = GTestCase{test};
      if (!test_case.match(pats)) continue;
      _matchs.push(test_case);
    }
    return _matchs.len();
  }

  void run(bool use_color) {
    _run_cnt = 0;
    _passed_cnt = 0;
    _failed_cnt = 0;
    _duration = {};

    this->on_run_start(use_color);
    const auto timer = time::Instant::now();
    for (auto& test_case : _matchs.as_mut_slice()) {
      _run_cnt += 1;

      const auto passed = test_case.run(use_color);
      if (passed) {
        _passed_cnt += 1;
      } else {
        _failed_cnt += 1;
      }
    }
    _duration = timer.elapsed();
    this->on_run_end(use_color);
  }

  void show_fails(bool use_color) const {
    const auto label_failed = ColoredStr{"[  FAILED  ]", Color::RED, use_color};

    for (const auto& test_case : _matchs.as_slice()) {
      if (test_case._stat == GTestStat::Failed) {
        io::println("{} {}", label_failed, test_case);
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

  void on_run_start(bool use_color) const {
    const auto label = ColoredStr{"[----------]", Color::GREEN, use_color};
    io::println("{} {} tests from {}", label, _matchs.len(), _mod.name());
  }

  void on_run_end(bool use_color) const {
    const auto label = ColoredStr{"[----------]", Color::GREEN, use_color};

    const auto test_cnt = _matchs.len();
    const auto mod_name = _mod._name;
    const auto dur_ms = _duration.as_millis();
    io::println("{} {} tests from {} ({} ms total)", label, test_cnt, mod_name, dur_ms);
    io::println("");
  }
};

GTestApp::GTestApp() {}

GTestApp::~GTestApp() {}

void GTestApp::show_help() {
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

void GTestApp::list(Str path) {
  if (path.starts_with("xml:")) {
    const auto xml_path = path[{4, $}];

    const auto xml_str = GTestApp::list_xml();
    fs::write(xml_path, xml_str.as_bytes()).expect(fmt::Args{"write xml to `{}` failed", xml_path});
    return;
  }

  const auto xml_str = GTestApp::list_xml();
  io::println("{}", xml_str);
}

void GTestApp::filt(Str filter) {
  auto pats = split_pats(filter);

  _suites.clear();
  _case_cnt = 0;
  for (auto& test_mod : test::mods()) {
    auto suite = GTestSuite{test_mod};
    const auto case_cnt = suite.filt(pats.as_slice());
    if (case_cnt == 0) {
      continue;
    }
    _case_cnt += case_cnt;
    _suites.push(mem::move(suite));
  }
}

auto GTestApp::run(bool use_color) -> bool {
  _run_cnt = 0;
  _passed_cnt = 0;
  _failed_cnt = 0;
  _duration_ms = 0;

  this->on_run_start(use_color);
  const auto timer = time::Instant::now();
  for (auto& suite : _suites.as_mut_slice()) {
    suite.run(use_color);
    _run_cnt += suite._run_cnt;
    _passed_cnt += suite._passed_cnt;
    _failed_cnt += suite._failed_cnt;
  }
  _duration_ms = timer.elapsed().as_millis();
  this->on_run_end(use_color);
  this->show_summary(use_color);

  return _failed_cnt == 0;
}

auto GTestApp::list_xml() -> String {
  const auto all_suites = test::mods();
  const auto total_cnt = all_suites.iter().fold(0UZ, [](usize n, auto& s) { return n + s.count(); });

  auto buf = String{};
  auto out = fmt::Formatter(buf);
  out.write_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  out.write_fmt("<testsuites tests=\"{}\" name=\"AllTests\">\n", total_cnt);
  for (auto& suite : all_suites) {
    auto tests = suite.tests();
    out.write_fmt("  <testsuite name=\"{}\" tests=\"{}\">\n", suite.name(), tests.len());
    for (auto& test : tests) {
      const auto name = test._name;
      const auto file = Str::from_cstr(test._loc.file);
      const auto line = test._loc.line;
      out.write_fmt("    <testcase name=\"{}\" file=\"{}\" line=\"{}\"/>\n", name, file, line);
    }
    out.write_str("  </testsuite>\n");
  }
  out.write_str("</testsuites>\n");
  return buf;
}

auto GTestApp::split_pats(Str filter) -> List<Str> {
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

void GTestApp::on_run_start(bool use_color) const {
  const auto label = ColoredStr{"[==========]", Color::GREEN, use_color};
  io::println("{} Running {} tests from {} test suites.", label, _case_cnt, _suites.len());
}

void GTestApp::on_run_end(bool use_color) const {
  const auto label = ColoredStr{"[==========]", Color::GREEN, use_color};
  io::println("{} tests from {} test suites ran. ({} ms total)", label, _run_cnt, _suites.len(), _duration_ms);
}

void GTestApp::show_summary(bool use_color) const {
  const auto pass_title = ColoredStr{"[  PASSED  ]", Color::GREEN, use_color};
  const auto fail_title = ColoredStr{"[  FAILED  ]", Color::RED, use_color};

  io::println("{} {} tests.", pass_title, _passed_cnt);

  if (_failed_cnt != 0) {
    io::println("{} {} tests. listed below:", fail_title, _failed_cnt);
    for (const auto& suite : _suites.as_slice()) {
      suite.show_fails(use_color);
    }
    io::println("");
    io::println("{} FAILED TESTS", fail_title);
  }
}

}  // namespace sfc::test
