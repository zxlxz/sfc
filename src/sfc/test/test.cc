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

Module::Module(Str name) : _name(name) {}

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

struct Ctx {
  List<Module> _mods;

  static auto instance() -> Ctx& {
    static auto ctx = Ctx{};
    return ctx;
  }

  auto get_mod(Str name) -> Module& {
    return _mods.iter_mut()
        .find([&](Module& m) { return m._name == name; })
        .unwrap_or_else([&] -> Module& { return _mods.push(Module{name}); });
  }

  void regist(Test test) {
    auto& mod = this->get_mod(test.mod());
    mod.regist(test);
  }
};

auto modules() noexcept -> Slice<const Module> {
  static auto& ctx = Ctx::instance();
  return ctx._mods.as_slice();
}

auto regist(Test test) noexcept -> bool {
  static auto& ctx = Ctx::instance();
  ctx.regist(test);
  return true;
}

}  // namespace sfc::test
