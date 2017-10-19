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

  public:
    struct Installer {
        str _type;
        str _name;

       fn install(void(*func)()) -> void;

        template<typename F>
        fn operator<<(F&& f) -> int {
            install(f);
            return 0;
        }
    };

    template<class T>
    Installer install(str func) {
        return { type_name<T>(), func };
    }
  private:
    Scheduler() = default;

    Scheduler(Scheduler&&)           = default;
    Scheduler(const Scheduler&)      = delete;
    void operator=(const Scheduler&) = delete;
};

inline fn scheduler() -> Scheduler& {
    return Scheduler::instance();
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

