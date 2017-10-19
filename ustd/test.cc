#include "config.h"

namespace ustd::test
{

using namespace ustd::vec;
using namespace ustd::string;
using namespace ustd::fmt;
using namespace ustd::io;

class Worker
{
  public:
    str _type;
    str _name;
    void(*_func)();

    enum State
    {
        Ignore,
        Running,
        Success,
        Error,
    };

    fn operator()(const strs& pattern) -> i32 {
        if (!match(pattern)) {
            on(Ignore);
            return 0;
        }

        on(Running);

        thread::spawn([=] { (_func)(); })
            .join()
            .and_then([&]       {on(Success); })
            .or_else ([&](int)  {on(Error);   });

        return 1;
    }

  private:
    fn match(const strs& patterns) -> bool {
        if (patterns.len() == 0) {
            return true;
        }

        int cnts = 0;
        for (let& pattern : patterns) {
            switch (pattern[0]) {
            case '+':
                if (_name.starts_with(pattern(1, -1))) {
                    ++cnts;
                }

            case '-':
                if (_name.starts_with(pattern(1, -1))) {
                    return false;
                }
            default:
                if (_name == pattern) {
                    return true;
                }
            }
        }

        return cnts != 0;
    }

    fn on(State stat) -> void {
        static const TermColor colors[] = { TermColor::YEL, TermColor::CYN, TermColor::GRN, TermColor::RED };
        static const char*     signs[]  ={ "--", ">>", "<<", "!!" };

        char text[256];
        let  text_len = io::stdout().is_term()
            ? snprintf(text, sizeof(text), "\033[3%cm[%s]\033[0m%.*s::%.*s", char(colors[stat]), signs[stat], _type.len(), _type.data(), _name.len(), _name.data())
            : snprintf(text, sizeof(text), "[%s]%.*s::%.*s", signs[stat], _type.len(), _type.data(), _name.len(), _name.data());

        if (text_len > 0) {
            let msg = str(text, u32(text_len));
            io::stdout().writeln(msg);
        }
    }
};

fn _get_tests() -> Vec<Worker>& {
    static mut tests = Vec<Worker, 1024>{};
    return tests;
}

fn Scheduler::Installer::install(void(*func)()) -> void {
    static mut& tests = _get_tests();
    tests.push(Worker{_type, _name, func});
}

fn Scheduler::invoke(strs pattern) -> int {
    let& tests = _get_tests();

    mut cnt = i32(0);
    for(auto& test: tests) {
        cnt += test(pattern);
    }

    return cnt;
}

}
