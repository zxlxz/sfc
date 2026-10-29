#include "sfc/test/test.h"

namespace sfc::test {

static auto unpack_mod_fun(Str type) -> Tuple<Str, Str> {
  if (type.is_empty()) {
    return {{}, {}};
  }

  // mod::_fun
  const auto p = type.rfind(':').unwrap_or(0);

  auto mod_name = p == 0 ? Str{} : type[{0, p - 1}];
  auto fun_name = p == 0 ? type : type[{p + 2, $}];
  if (fun_name[0] == '_') {
    fun_name = fun_name[{1, $}];
  }
  return {mod_name, fun_name};
}

auto TestFn::run() const noexcept -> bool {
  if (_func == nullptr) {
    return false;
  }

  try {
    _func();
  } catch (...) {
    return false;
  }

  return true;
}

TestMod::TestMod(Str name) : _name{name} {}

TestMod::~TestMod() {}

TestMod::TestMod(TestMod&&) noexcept = default;

TestMod& TestMod::operator=(TestMod&&) noexcept = default;

auto TestMod::name() const noexcept -> Str {
  return _name;
}

auto TestMod::count() const noexcept -> usize {
  return _tests.len();
}

auto TestMod::tests() const noexcept -> Slice<const TestFn> {
  return _tests.as_slice();
}

void TestMod::regist(TestFn test) {
  _tests.push(test);
}

struct TestManager {
  List<TestMod> _mods{};

 public:
  static auto instance() -> TestManager& {
    static auto ctx = TestManager{};
    return ctx;
  }

  auto mods() const -> Slice<const TestMod> {
    return _mods.as_slice();
  }

  auto regist_mod(Str mod) -> TestMod& {
    for (auto& m : _mods.as_mut_slice()) {
      if (m._name == mod) return m;
    }

    auto& ret = _mods.push(TestMod{mod});
    return ret;
  }

  void regist(TestFn test) {
    auto& mod = this->regist_mod(test._mod);
    mod.regist(test);
  }
};

auto mods() noexcept -> Slice<const TestMod> {
  static auto& ctx = TestManager::instance();
  return ctx.mods();
}

auto regist_test(Str type, Loc loc, void (*fun)()) noexcept -> bool {
  static auto& ctx = TestManager::instance();

  const auto [mod_name, fun_name] = unpack_mod_fun(type);
  ctx.regist(TestFn{mod_name, fun_name, loc, fun});
  return true;
}

}  // namespace sfc::test
