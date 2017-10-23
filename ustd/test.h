#pragma once

#include <ustd/core.h>

namespace ustd::test
{

class Scheduler
{
  public:
    fn invoke(strs pattern) -> int;

    static fn instance() -> Scheduler& {
        static mut ret = Scheduler{};
        return ret;
    }

    template<typename T, int Id>
    struct Installer
    {
        Installer(str name, void(*func)()) {
            _id = Scheduler::instance().install(type_name<T>(), name, func);
        }

        fn id() const {
            return _id;
        }

    private:
        int _id;
    };

  private:

    Scheduler() = default;

    Scheduler(Scheduler&&)           = default;
    Scheduler(const Scheduler&)      = delete;
    void operator=(const Scheduler&) = delete;

    fn install(str type, str name, void(*func)()) -> int;
};

template<typename T, int Id>
using Installer = Scheduler::Installer<T, Id>;

template<class T, int Id>
int install(str name, void(*func)()) {
    static Scheduler::Installer<T, Id> obj(name, func);
    return obj.id();
}

template<class ...T>
fn (assert)(bool val, const T& ...args) -> void {
    if (val) {
        return;
    }
    ustd::panic(args...);
}

template<class Ta, class Tb, class ...T>
fn (assert_eq)(const Ta& a, const Tb& b, const T& ...args) -> void {
    (assert)(a == b, args...);
}

template<class Ta, class Tb, class ...T>
fn (assert_ne)(const Ta& a, const Tb& b, const T& ...args) -> void {
    (assert)(a != b, args...);
}

}

