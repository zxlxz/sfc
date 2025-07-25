#include "sfc/test/app.h"

#include "sfc/fs.h"
#include "sfc/io.h"
#include "sfc/time.h"

namespace sfc::test {

static auto parse_opts(Slice<const Str> args) -> VecMap<Str, Str> {
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

static auto yes_or_no(Str s, bool default_val) -> bool {
  if (s == "yes" || s == "true" || s == "on" || s == "1") {
    return true;
  }
  if (s == "no" || s == "false" || s == "off" || s == "0") {
    return false;
  }
  return default_val;
}

App::App() {}

App::~App() {}

App::App(App&&) noexcept = default;

auto App::run(Slice<const Str> args) -> int {
  auto opts = parse_opts(args);

  if (opts.contains_key("help") || opts.contains_key("h")) {
    this->help();
    return 0;
  }

  if (opts.contains_key("gtest_list_tests")) {
    auto path = opts.get("gtest_output").unwrap_or("");
    this->list_to_file(path);
    return 0;
  }

  const auto pats = opts.get("gtest_filter").unwrap_or("");
  const auto color_str = opts.get("gtest_color").unwrap_or("");
  const auto color_val = yes_or_no(color_str, io::Stdout::is_tty());
  this->run_tests(pats, color_val);

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
  auto& tester = Tester::instance();
  tester.invoke({}, color);
}

void App::list_to_file(Str output) const {
  const auto file_indx = output.find(':');
  const auto file_type = file_indx ? output[{0, *file_indx}] : Str{};
  const auto file_path = file_indx ? output[{*file_indx + 1, _}] : output;

  const auto file_text = file_type == Str{"xml"} ? this->list_tests_xml() : this->list_tests();
  if (file_path) {
    auto file = fs::File::create(fs::Path::from(file_path)).unwrap();
    file.write_str(file_text.as_str());
  } else {
    io::Stdout{}.write_str(file_text.as_str());
  }
}

auto App::list_tests() const -> String {
  auto suites = Tester::instance().suites();

  auto sbuf = String{};
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
  auto suites = Tester::instance().suites();
  auto test_cnt = suites.iter().fold(0UL, [](auto n, auto& x) { return n += x.tests().len(); });

  auto sbuf = String{};
  fmt::Fmter<String> f{sbuf};
  f.write_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  f.write_fmt("<testsuites tests=\"{}\" name=\"AllTests\">\n", test_cnt);
  for (auto& suite : suites) {
    auto tests = suite.tests();
    f.write_fmt("  <testsuite name=\"{}\" tests=\"{}\">\n", suite.name(), tests.len());
    for (auto& test : tests) {
      const auto name = test.name();
      const auto loc = test._loc;
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
