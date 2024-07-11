#include "unit.h"

namespace sfc::test {

auto Unit::suite() const -> Str {
  const auto p = _name.rfind(':');
  return p ? _name[{0, *p - 1}] : Str{};
}

auto Unit::name() const -> Str {
  const auto p = _name.rfind(':');
  const auto s = p ? _name[{*p + 1, _}] : _name;
  return s[{0, s.len() - sizeof("_SFCT")}];
}

auto Unit::location() const -> Location {
  return _loc;
}

auto Unit::invoke() const -> bool {
  try {
    (_func)();
    return true;
  } catch (...) {
    return false;
  }
}

auto Unit::match(Slice<const Str> pats) const -> bool {
  if (pats.is_empty()) {
    return true;
  }

  const auto suite = this->suite();
  const auto name = this->name();

  for (auto pat : pats) {
    const auto pat_pairs = pat.split_at(suite.len());
    const auto pat_suite = pat_pairs.template get<0>();
    const auto pat_name = pat_pairs.template get<1>()[{1, _}];

    if (!suite.starts_with(pat_suite)) {
      continue;
    }

    if (pat_name.is_empty()) {
      return true;
    }

    if (pat_name == name) {
      return true;
    }
  }

  return false;
}

Suite::Suite(Str name) : _name{name} {}

Suite::~Suite() {}

Suite::Suite(Suite&&) noexcept = default;

Suite& Suite::operator=(Suite&&) noexcept = default;

auto Suite::name() const -> Str {
  return _name;
}

void Suite::push(Unit val) {
  _tests.push(val);
}

auto Suite::tests() const -> Slice<const Unit> {
  return _tests.as_slice();
}

auto Suite::match(Slice<const Str> pats) const -> bool {
  if (!pats) return true;

  auto get_suite_name = [](Str s) {
    const auto p = s.find('.').unwrap_or(s._len);
    return s[{0, p}];
  };

  for (auto pat : pats) {
    auto pat_suite = get_suite_name(pat);
    if (_name.starts_with(pat_suite)) {
      if (_name.len() == pat_suite.len() || _name[pat_suite.len()] == ':') {
        return true;
      }
    }
  }
  return false;
}

TestManager::TestManager() {}

TestManager::~TestManager() {}

auto TestManager::instance() -> TestManager& {
  static TestManager res{};
  return res;
}

auto TestManager::suites() const -> Slice<const Suite> {
  return _suites.vals();
}

auto TestManager::units(Slice<const Str> pats) const -> Vec<Unit> {
  auto res = Vec<Unit>{};
  for (auto& suite : _suites.vals()) {
    if (!suite.match(pats)) {
      continue;
    }
    for (auto& test : suite.tests()) {
      if (test.match(pats)) {
        res.push(test);
      }
    }
  }

  return res;
}

auto TestManager::regist_imp(Unit val) -> usize {
  static usize COUNT = 0U;
  COUNT += 1;

  auto suite_name = val.suite();
  if (auto suite = _suites.get_mut(suite_name)) {
    (*suite).push(val);
    return COUNT;
  }

  auto new_suite = Suite{suite_name};
  new_suite.push(val);
  _suites.insert(suite_name, mem::move(new_suite));
  return COUNT;
}

}  // namespace sfc::test
