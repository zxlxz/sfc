#include "sfc/test/gtest.h"
#include "sfc/test/test.h"
#include "sfc/io/stdio.h"
#include "sfc/time.h"
#include "sfc/fs.h"

namespace sfc::test::gtest {

using time::Duration;

namespace {

enum class Color {
  RED = 1,
  GREEN = 2,
  YELLOW = 3,
  BLUE = 4,
  MAGENTA = 5,
  CYAN = 6,
  RESET = 7,
};

bool g_use_color = false;

auto fg(Color c) -> Str {
  if (!g_use_color) {
    return "";
  }

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

}  // namespace

class GTest {
  Test* _inn{nullptr};

 public:
  GTest(Test& inn) : _inn(&inn) {}

 public:
  auto status() const -> Status {
    return _inn->status();
  }

  auto location() const -> SourceLoc {
    return _inn->location();
  }

  auto suite_name() const -> Str {
    return _inn->mod_name();
  }

  auto test_name() const -> Str {
    return _inn->func_name();
  }

  void filt(Slice<const Str> pats) {
    const auto ret = this->match_any(pats);
    _inn->set_status(ret ? Status::Ready : Status::None);
  }

  auto run() -> bool {
    const auto color_run = fg(Color::GREEN);
    const auto color_ok = fg(Color::GREEN);
    const auto color_fail = fg(Color::RED);
    const auto color_reset = fg(Color::RESET);

    const auto timer = time::Instant::now();
    io::println("{}[ RUN      ]{} {}", color_run, color_reset, *this);
    const auto ret = _inn->run();
    const auto dur = timer.elapsed();
    if (ret) {
      io::println("{}[       OK ]{} {} ({} ms)", color_ok, color_reset, *this, dur.as_millis());
    } else {
      io::println("{}[  FAILED  ]{} {} ({} ms)", color_fail, color_reset, *this, dur.as_millis());
    }
    return ret;
  }

  void fmt(auto& f) const {
    const auto mod = _inn->mod_name();
    const auto fun = _inn->func_name();

    if (mod.is_empty()) {
      f.write_fmt("{}", fun);
    } else {
      f.write_fmt("{}.{}", mod, fun);
    }
  }

 private:
  auto match(Str pat) const -> bool {
    if (pat.is_empty()) {
      return true;
    }

    const auto mod_name = _inn->mod_name();
    const auto func_name = _inn->func_name();

    // match: func
    if (mod_name.is_empty()) {
      return pat == func_name;
    }

    // match: mod.*
    if (pat.ends_with(".*")) {
      const auto mod_part = pat[{0, pat.len() - 2}];
      return mod_name == mod_part;
    }

    // match: mod.func
    if (pat.len() == mod_name.len() + 1 + func_name.len()) {
      if (pat[mod_name.len()] != '.') {
        return false;
      }
      const auto mod_part = pat[{0, mod_name.len()}];
      const auto func_part = pat[{mod_name.len() + 1, $}];
      return mod_name == mod_part && func_name == func_part;
    }

    return false;
  }

  auto match_any(Slice<const Str> pats) const -> bool {
    if (pats.is_empty()) {
      return true;
    }

    for (auto pat : pats) {
      if (this->match(pat)) {
        return true;
      }
    }
    return false;
  }
};

class Suite {
  Str _name;
  List<GTest> _tests;
  time::Duration _duration{};

  Suite(Str name, List<GTest> tests) : _name{name}, _tests{mem::move(tests)} {}

 public:
  static auto new_(Str name) -> Suite {
    return Suite{name, List<GTest>::with_capacity(64)};
  }

 public:
  auto name() const -> Str {
    return _name;
  }

  auto count(Status stat) const -> usize {
    usize cnt = 0;
    for (auto& test : _tests) {
      cnt += test.status() == stat ? 1 : 0;
    }
    return cnt;
  }

  auto tests() const -> Slice<const GTest> {
    return _tests.as_slice();
  }

  void filt(Slice<const Str> pats) {
    for (auto& gtest : _tests) {
      gtest.filt(pats);
    }
  }

  void push(Test& test) {
    _tests.push({test});
  }

  void run() {
    const auto color_run = fg(Color::GREEN);
    const auto color_off = fg(Color::RESET);

    const auto ready_cnt = this->count(Status::Ready);
    if (ready_cnt == 0) {
      return;
    }

    io::println("{}[----------]{} {} tests from {}", color_run, color_off, ready_cnt, _name);
    const auto timer = time::Instant::now();
    for (auto& test : _tests) {
      if (test.status() == Status::Ready) {
        test.run();
      }
    }
    _duration = timer.elapsed();

    const auto dur_ms = _duration.as_millis();
    io::println("{}[----------]{} {} tests from {} ({} ms total)", color_run, color_off, ready_cnt, _name, dur_ms);
    io::println("");
  }

  void show_fails() const {
    const auto color_err = fg(Color::RED);
    const auto color_off = fg(Color::RESET);

    const auto failed_cnt = this->count(Status::Failed);
    if (failed_cnt == 0) {
      return;
    }

    for (auto& test : _tests) {
      if (test.status() == Status::Failed) {
        io::println("{}[  FAILED  ]{} {}", color_err, color_off, test);
      }
    }
  }
};

class App {
  List<Suite> _suites{};

  explicit App(List<Suite> suites) : _suites{mem::move(suites)} {}

 public:
  static auto new_() -> App {
    auto res = App{List<Suite>::with_capacity(256U)};

    test::for_each(&res, [](void* ctx, Test& test) {
      auto* self = (App*)(ctx);
      self->push(test);
    });
    return res;
  }

 public:
  auto suites() const -> Slice<const Suite> {
    return _suites.as_slice();
  }

  auto suite_count() const -> usize {
    usize cnt = 0;
    for (auto& suite : _suites) {
      const auto ready_cnt = suite.count(Status::Ready);
      cnt += ready_cnt == 0 ? 0 : 1;
    }
    return cnt;
  }

  auto test_count(Status stat) const -> usize {
    usize cnt = 0;
    for (const auto& suite : _suites) {
      cnt += suite.count(stat);
    }
    return cnt;
  }

  void filt(Slice<const Str> pats) {
    for (auto& suite : _suites) {
      suite.filt(pats);
    }
  }

  auto total_count() const -> usize {
    usize cnt = 0UZ;
    for (const auto& suite : _suites) {
      cnt += suite.tests().len();
    }
    return cnt;
  }

  auto run() -> bool {
    const auto color_run = fg(Color::GREEN);
    const auto color_err = fg(Color::RED);
    const auto color_off = fg(Color::RESET);

    const auto suite_cnt = this->suite_count();
    const auto ready_cnt = this->test_count(Status::Ready);

    io::println("{}[==========]{} Running {} tests from {} test suites.", color_run, color_off, ready_cnt, suite_cnt);
    const auto timer = time::Instant::now();
    for (auto& suite : _suites) {
      suite.run();
    }
    const auto dur_ms = timer.elapsed().as_millis();
    io::println("{}[==========]{} Running {} tests from {} test suites. ({} ms total)",
                color_run,
                color_off,
                ready_cnt,
                suite_cnt,
                dur_ms);

    const auto passed_cnt = this->test_count(Status::Passed);
    io::println("{}[  PASSED  ]{} {} tests.", color_run, color_off, passed_cnt);

    const auto failed_cnt = this->test_count(Status::Failed);
    if (failed_cnt != 0) {
      io::println("{}[  FAILED  ]{} {} tests. listed below:", color_err, color_off, failed_cnt);
      for (const auto& suite : _suites) {
        suite.show_fails();
      }
      io::println("");
      io::println("{}[  FAILED  ]{} {} FAILED TESTS", color_err, color_off, failed_cnt);
    }

    return failed_cnt == 0;
  }

 private:
  auto suite(Str suite_name) -> Suite& {
    for (auto& suite : _suites) {
      if (suite.name() == suite_name) {
        return suite;
      }
    }
    return _suites.push(Suite::new_(suite_name));
  }

  void push(Test& test) {
    auto& suite = this->suite(test.mod_name());
    suite.push(test);
  }
};

void list_xml(auto& out) {
  auto app = App::new_();

  const auto total_cnt = app.total_count();

  out.write_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  out.write_fmt("<testsuites tests=\"{}\" name=\"AllTests\">\n", total_cnt);
  for (const auto& suite : app.suites()) {
    const auto tests = suite.tests();
    out.write_fmt("  <testsuite name=\"{}\" tests=\"{}\">\n", suite.name(), tests.len());
    for (auto& test : tests) {
      const auto name = test.test_name();
      const auto loc = test.location();
      out.write_fmt("    <testcase name=\"{}\" file=\"{}\" line=\"{}\"/>\n", name, loc.file, loc.line);
    }
    out.write_str("  </testsuite>\n");
  }
  out.write_str("</testsuites>\n");
}

auto split_pats(Str filter) -> List<Str> {
  auto find_pos = [](Str s) -> usize {
    const auto n = s._len;
    for (auto p = 0U; p < n; p++) {
      switch (s[p]) {
        case ';': {
          return p;
        }
        case ':': {
          if (p + 1 < n && s[p + 1] == ':') {
            p += 1;  // skip the second '::'
            continue;
          }
          return p;  // single ':' found
        }
        default: break;
      }
    }
    return s._len;
  };

  auto pats = List<Str>::with_capacity(32);
  while (!filter.is_empty()) {
    const auto pos = find_pos(filter);
    const auto pat = filter[{0, pos}];
    if (!pat.is_empty()) {
      pats.push(pat);
    }
    filter = filter[{pos + 1, $}];
  }
  return pats;
}

int run(Slice<const Str> pats) {
  auto app = App::new_();
  app.filt(pats);
  app.run();

  const auto failed_cnt = app.test_count(Status::Failed);
  return failed_cnt == 0 ? 0 : 1;
}

}  // namespace sfc::test::gtest

namespace sfc::test {
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

void gtest_list(Str uri) {
  if (uri.starts_with("xml:")) {
    const auto xml_path = uri[{4, $}];

    auto xml_file = fs::File::create(xml_path).expect(fmt::Args{"create xml file `{}` failed", xml_path});
    auto xml_fmter = fmt::Formatter(xml_file);
    gtest::list_xml(xml_fmter);
    return;
  }

  auto stdout_lock = io::Stdout{}.lock();
  auto stdout_fmter = fmt::Formatter(stdout_lock);
  gtest::list_xml(stdout_fmter);
}

auto gtest_run(Str pattern, bool use_color) -> int {
  gtest::g_use_color = use_color;

  const auto pats = gtest::split_pats(pattern);
  return gtest::run(pats.as_slice());
}

}  // namespace sfc::test
