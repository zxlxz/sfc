#include "sfc/test/test.h"

namespace sfc::test {

auto Test::from(Str type, func_t func, SourceLoc loc) -> Test {
  const auto p = type.rfind(':').unwrap_or(0);
  if (p == 0) {
    return Test{"", type[{1, $}], func, loc};
  }
  const auto mod = type[{0, p - 1}];
  const auto name = type[{p + 2, $}];
  return Test{mod, name, func, loc};
}

auto Test::mod() const noexcept -> Str {
  return _mod;
}

auto Test::name() const noexcept -> Str {
  return _name;
}

void Test::run() const {
  if (_func == nullptr) return;
  _func();
}

Module::Module(Str name) : _name{name} {}

Module::~Module() {}

Module::Module(Module&&) noexcept = default;

Module& Module::operator=(Module&&) noexcept = default;

auto Module::name() const noexcept -> Str {
  return _name;
}

auto Module::count() const noexcept -> usize {
  return _tests.len();
}

auto Module::tests() const noexcept -> Slice<const Test> {
  return _tests.as_slice();
}

void Module::regist(Test test) {
  _tests.push(test);
}

struct TestManager {
  List<Module> _mods{};

 public:
  static auto instance() -> TestManager& {
    static auto ctx = TestManager{};
    return ctx;
  }

  auto mods() const -> Slice<const Module> {
    return _mods.as_slice();
  }

  auto regist_mod(Str mod) -> Module& {
    for (auto& m : _mods.as_mut_slice()) {
      if (m._name == mod) return m;
    }

    auto& ret = _mods.push(Module{mod});
    return ret;
  }

  void regist(Test test) {
    auto& mod = this->regist_mod(test._mod);
    mod.regist(test);
  }
};

auto modules() noexcept -> Slice<const Module> {
  static auto& ctx = TestManager::instance();
  return ctx.mods();
}

auto regist(Test test) noexcept -> bool {
  static auto& ctx = TestManager::instance();
  ctx.regist(test);
  return true;
}

}  // namespace sfc::test
