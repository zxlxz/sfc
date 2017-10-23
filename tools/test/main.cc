#include "ustd.h"

using namespace ustd;

int main(int argc, const char* argv[]) {
    mut args = vec::Vec<str, 64>();
    for (int i = 1; i < argc; ++i) {
        let s = str::from_raw(argv[i]);
        args.push(s);
    }
    test::Scheduler::instance().invoke(args);

    return 0;
}

