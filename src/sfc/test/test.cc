#include "sfc/test/test.h"

namespace sfc::test {

auto Test::mod_name() const noexcept -> Str {
  // mod::_class -> mod
  const auto colon_pos = _type.rfind(':').unwrap_or(0);
  if (colon_pos == 0) {
    return Str{};
  }

  const auto n = colon_pos - 1;
  return _type[{0, n}];
}

auto Test::func_name() const noexcept -> Str {
  if (_type.is_empty()) {
    return {};
  }

  // mod::_class -> _class
  const auto colon_pos = _type.rfind(':').unwrap_or(0);
  if (colon_pos == 0) {
    return _type[{1, $}];
  }

  // _class -> class
  const auto cls_name = _type[{colon_pos + 1, $}];
  const auto func_name = cls_name.starts_with('_') ? cls_name[{1, $}] : cls_name;
  return func_name;
}

void Test::set_status(Status stat) {
  _stat = stat;
}

auto Test::run() noexcept -> bool {
  if (_stat != Status::Ready) {
    return false;
  }

  if (_func == nullptr) {
    this->set_status(Status::Failed);
    return false;
  }

  try {
    this->set_status(Status::Running);
    _func();
    this->set_status(Status::Passed);
  } catch (...) {
    this->set_status(Status::Failed);
  }

  return _stat == Status::Passed;
}

auto Test::status() const noexcept -> Status {
  return _stat;
}

auto Test::location() const noexcept -> Loc {
  return _loc;
}

struct TestList {
  Test* _head = nullptr;
  Test* _tail = nullptr;

 public:
  static auto instance() -> TestList& {
    static auto list = TestList{};
    return list;
  }

 public:
  void push(Test& test) {
    if (_tail == nullptr) {
      _head = &test;
      _tail = &test;
      return;
    }

    _tail->_next = &test;
    _tail = &test;
  }

  void for_each(auto&& f) {
    if (_head == nullptr) {
      return;
    }

    for (auto* test = _head; test != nullptr; test = test->_next) {
      f(*test);
    }
  }
};

void regist_imp(Test& test) {
  auto& glist = TestList::instance();
  glist.push(test);
}

void for_each(void* ctx, void (*f)(void* ctx, Test& test)) {
  auto& glist = TestList::instance();
  glist.for_each([f, ctx](Test& test) { f(ctx, test); });
}

}  // namespace sfc::test
