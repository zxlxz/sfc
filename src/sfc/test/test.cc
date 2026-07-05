#include "sfc/test/test.h"

namespace sfc::test {

static auto unpack_test_name(Str type) -> Tuple<Str, Str> {
  if (type.is_empty()) {
    return {{}, {}};
  }

  // a::b::c::_test_name
  const auto p = type.rfind(':').unwrap_or(0);
  const auto mod_name = p == 0 ? Str{} : type[{0, p - 1}];
  const auto class_name = p == 0 ? type : type[{p + 2, $}];
  const auto type_name = class_name[0] == '_' ? class_name[{1, $}] : class_name;

  return {mod_name, type_name};
}

auto Test::mod() const noexcept -> Str {
  const auto mod_name = test::unpack_test_name(_path);
  return mod_name._0;
}

auto Test::name() const noexcept -> Str {
  const auto mod_name = test::unpack_test_name(_path);
  return mod_name._1;
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
    auto& mod = this->regist_mod(test.mod());
    mod.regist(test);
  }
};

auto modules() noexcept -> Slice<const Module> {
  static auto& ctx = TestManager::instance();
  return ctx.mods();
}

auto regist_test(Test test) noexcept -> bool {
  static auto& ctx = TestManager::instance();
  ctx.regist(test);
  return true;
}

}  // namespace sfc::test
