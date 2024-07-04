#include "sfc/app/opts.h"
#include "sfc/fs.h"
#include "sfc/io.h"
#include "sfc/serde/xml.h"
#include "sfc/test.h"

namespace sfc::test {

static auto parse_filter(Str filter) -> Vec<Str> {
  auto find_pos = [](Str s) -> usize {
    for (auto p = 0U; p < s._len; p++) {
      switch (s[p]) {
        case ';': return p;
        case ':':
          if (s[p + 1] != ':') {
            return p;
          }
          p++;
      }
    }
    return s._len;
  };

  auto v = Vec<Str>{};
  while (filter) {
    const auto pos = find_pos(filter);
    v.push(filter[{0, pos}]);
    filter = filter[{pos + 1, _}];
  }
  return v;
}

static auto format_xml(Slice<const Suite> suites) -> String {
  auto all_cnt = 0U;
  for (const auto& suite : suites) {
    all_cnt += suite.tests().len();
  }

  auto testsuites = serde::XmlNode{"testsuites"};
  testsuites.add_attr("tests", string::to_string(all_cnt));
  testsuites.add_attr("name", "AllTests");

  for (auto& suite : suites) {
    auto& testsuite = testsuites.add_node(serde::XmlNode{"testsuite"});
    testsuite.add_attr("name", suite.name());
    testsuite.add_attr("tests", string::to_string(suite.tests().len()));
    for (auto& test : suite.tests()) {
      auto& testcase = testsuite.add_node(serde::XmlNode{"testcase"});
      testcase.add_attr("name", test.name());
      testcase.add_attr("file", Str::from(test._loc.file));
      testcase.add_attr("line", string::to_string(test._loc.line));
    }
  }

  auto xml_str = testsuites.to_string();
  return xml_str;
}

App::App() : _suites{_suites.with_capacity(64)} {}

App::~App() {}

auto App::operator[](Str name) -> Suite& {
  for (auto& suite : _suites.as_mut_slice()) {
    if (suite.name() == name) {
      return suite;
    }
  }
  return _suites.push(Suite{name});
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

void App::run(Str filter, Option<bool> color_opt) {
  const auto pats = parse_filter(filter);
  const auto color = color_opt ? *color_opt : io::Stdout::is_tty();

  for (auto& suite : _suites.as_mut_slice()) {
    suite.run_tests(pats.as_slice(), color);
  }
}

void App::list() const {
  auto suites = _suites.as_slice();

  auto outs = io::Stdout::lock();
  for (auto& suite : suites) {
    outs.write_fmt("{}\n", suite.name());
    for (auto& test : suite.tests()) {
      outs.write_fmt("    {}\n", test.name());
    }
  }
}

void App::list_xml(Str path) const {
  const auto xml_str = format_xml(_suites.as_slice());
  if (!path || path == "stdout") {
    io::println(xml_str);
    return;
  }

  const auto pos = path.find(':');
  const auto xml_path = pos ? path[{*pos + 1, _}] : path;
  auto xml_file = fs::File::create(fs::Path::from(xml_path)).ok();
  if (!xml_file) {
    io::println("Failed to create file '{}'", xml_path);
    return;
  }
  xml_file->write_str(xml_str);
}

auto App::instance() -> App& {
  static auto res = App{};
  return res;
}

}  // namespace sfc::test
