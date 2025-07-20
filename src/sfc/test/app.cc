#include "sfc/test/app.h"

#include "sfc/fs.h"
#include "sfc/io.h"
#include "sfc/time.h"

namespace sfc::test {

namespace {

class GTestCase {
  bool _color = false;

 public:
  explicit GTestCase(bool use_color) : _color{use_color} {}

  ~GTestCase() {}

  auto parse_pats(Str s) const -> Vec<Str> {
    auto res = Vec<Str>{};
    for (; s;) {
      const auto n = s.len();
      auto       i = 0U;
      for (; i < n; ++i) {
        if (s[i] == ':') {
          if (s[i + 1] == ':') {
            i += 1;
            continue;
          }
          break;
        }
      }

      res.push(s[{0, i}]);
      s = s[{i + 1, _}];
    }

    return res;
  }

  auto operator()(Unit test) -> bool {
    auto na = test.suite();
    auto nb = test.name();

    this->on_run(na, nb);
    const auto tim = time::Instant::now();
    const auto res = test.invoke();
    const auto dur = tim.elpased();
    const auto tms = dur.as_millis();
    if (res) {
      this->on_ok(na, nb, tms);
    } else {
      this->on_failed(na, nb, tms);
    }
    return res;
  }

 private:
  void on_run(Str suite, Str name) {
    const auto s = _color ? Str{"\e[32m[ RUN      ]\e[39m"} : Str{"[ RUN      ]"};
    io::println("{} {}.{}", s, suite, name);
  }

  void on_ok(Str suite, Str name, u64 tms) {
    const auto s = _color ? Str{"\e[32m[       OK ]\e[39m"} : Str{"[       OK ]"};
    io::println("{} {}.{} ({} ms)", s, suite, name, tms);
  }

  void on_failed(Str suite, Str name, u64 tms) {
    const auto s = _color ? Str{"\e[31m[  FAILED  ]\e[39m"} : Str{"[  FAILED  ]"};
    io::println("{} {}.{} ({} ms)", s, suite, name, tms);
  }
};

class CmdLine {
 public:
  auto parse(Slice<const Str> args) const -> VecMap<Str, Str> {
    auto opts = VecMap<Str, Str>{};

    auto prev_key = Str{};

    for (auto arg : args) {
      if (arg.starts_with("-")) {
        if (prev_key) {
          opts.insert(prev_key, "");
          prev_key = {};
        }

        if (arg.starts_with("--")) {
          arg = arg[{2, _}];
          if (auto p = arg.find('=')) {
            auto k = arg[{0, *p}];
            auto v = arg[{*p + 1, _}];
            opts.insert(k, v);
          } else {
            prev_key = arg;
          }
        } else {
          auto k = arg[{1, _}];
          opts.insert(k, "");
        }
      } else {
        if (prev_key) {
          opts.insert(prev_key, arg);
          prev_key = {};
        }
      }
    }
    if (prev_key) {
      opts.insert(prev_key, "");
    }

    return opts;
  }
};

}  // namespace

App::App() {}

App::~App() {}

App::App(App&&) noexcept = default;

auto App::run(Slice<const Str> args) -> int {
  auto opts = CmdLine{}.parse(args);

  if (opts.contains_key(Str{"help"}) || opts.contains_key(Str{"h"})) {
    this->help();
    return 0;
  }

  if (opts.contains_key(Str{"gtest_list_tests"})) {
    auto path = opts.get(Str{"gtest_output"}).unwrap_or("");
    this->list_to_file(path);
    return 0;
  }

  auto filter = opts.get(Str{"gtest_filter"}).unwrap_or("");
  auto color = [&]() {
    const auto opt = opts.get(Str{"gtest_color"}).unwrap_or("auto");
    if (opt == "yes") {
      return true;
    }
    if (opt == "no") {
      return false;
    }
    if (opt == "auto") {
      return io::Stdout{}.is_tty();
    }
    return false;
  }();

  this->run_tests(filter, color);

  return 0;
}

void App::help() {
  static const char msg[] =
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

void App::run_tests(Str filter, bool color) {
  auto gtest = GTestCase{color};
  auto pats = gtest.parse_pats(filter);

  auto tests = TestManager::instance().units(pats.as_slice());
  for (auto& x : tests.as_slice()) {
    gtest(x);
  }
}

void App::list_to_file(Str output) const {
  const auto type_indx = output.find(':');
  const auto file_type = output[{0, type_indx.unwrap_or(0)}];
  const auto file_path = output[{type_indx.unwrap_or(output.len() - 1) + 1, _}];

  const auto file_text = file_type == Str{"xml"} ? this->list_tests_xml() : this->list_tests();
  if (file_path) {
    auto file = fs::File::create(fs::Path::from(file_path)).unwrap();
    file.write_str(file_text.as_str());
  } else {
    io::Stdout{}.write_str(file_text.as_str());
  }
}

auto App::list_tests() const -> String {
  auto suites = TestManager::instance().suites();

  auto               sbuf = String{};
  fmt::Fmter<String> f{sbuf};
  for (const auto& suite : suites) {
    f.write_fmt("{}.\n", suite.name());
    for (const auto& test : suite.tests()) {
      f.write_fmt("  {}\n", test.name());
    }
  }
  return sbuf;
}

auto App::list_tests_xml() const -> String {
  auto suites = TestManager::instance().suites();
  auto test_cnt = suites.iter().fold(0UL, [](auto n, auto& x) { return n += x.tests().len(); });

  auto               sbuf = String{};
  fmt::Fmter<String> f{sbuf};
  f.write_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  f.write_fmt("<testsuites tests=\"{}\" name=\"AllTests\">\n", test_cnt);
  for (auto& suite : suites) {
    auto tests = suite.tests();
    f.write_fmt("  <testsuite name=\"{}\" tests=\"{}\">\n", suite.name(), tests.len());
    for (auto& test : tests) {
      const auto name = test.name();
      const auto loc = test.location();
      f.write_fmt("    <testcase name=\"{}\" file=\"{}\" line=\"{}\" />\n",
                  name,
                  loc.file,
                  loc.line);
    }
    f.write_str("  </testsuite>\n");
  }
  f.write_str("</testsuites>\n");
  return sbuf;
}

}  // namespace sfc::test
