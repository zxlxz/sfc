
#include <nms/test.h>
#include <nms/io.h>
#include <nms/util/stacktrace.h>

namespace nms
{
const ProcStacks& gExceptionStacks();
}

namespace nms::test
{

using namespace nms::io;

struct Testor
{
    StrView name;
    void    (*func)();

    bool match(const StrView& mask) const {
        if (mask.isEmpty()) return true;

        if (mask[0] == '-') {
            auto stat = match(mask.slice(1, -1));
            return !stat;
        }

        if (mask.count() > name.count()) {
            return false;
        }

        auto test = name.slice(0u, mask.count() - 1);
        if (mask == test) {
            return true;
        }

        return false;
    }

    bool match(const StrView masks[], u32 count) const {
        if (count == 0) return true;

        for (u32 i = 0; i < count; ++i) {
            if (match(masks[i])) return true;
        }
        return false;
    }

    bool invoke(bool stat) const {
        const char fmt_run[]    = "\033[1;36m[>>]{:7.3} {}\033[0m";
        const char fmt_ok[]     = "\033[1;32m[<<]{:7.3} {}\033[0m";
        const char fmt_fail[]   = "\033[1;31m[<<]{:7.3} {}\033[0m";
        const char fmt_pass[]   = "\033[1;33m[--]{:7.3} {}\033[0m";
        if (!stat) {
            console::writeln(fmt_pass, clock(), name);
            return false;
        }

        console::writeln(fmt_run, clock(), name);
        try {
            func();
            console::writeln(fmt_ok, clock(), name);
        }
        catch (const IException& e) {
            log::error("throw nms::IException {}", e);
            {
                String str;
                auto& stacks = gExceptionStacks();

                const auto stacks_cnt = stacks.count();
                for (auto i = 4u; i < 64 && i + 6 < stacks_cnt; ++i) {
                    sformat(str, "\t|-[{:2}] -> {}\n", i, stacks[i]);
                }
                console::writeln(str);
            }

            console::writeln(fmt_fail, clock(), name);
            return false;
        }
        catch (...) {
            return false;
        }
        return true;
    }
};

static auto& gTests() {
    static List<Testor> tests;
    return tests;
}

NMS_API u32 install(StrView name, void(*func)()) {
    static auto& gtests = gTests();
#if defined(NMS_CC_MSVC)
    Testor testor{ name.slice(i32(sizeof("auto __cdecl ")), -i32(sizeof("_func(void)"))), func };
#elif defined(NMS_CC_CLANG)
    // "auto "   : 5
    // "_func( )": 8
    Testor testor{ name.slice(5, -8), func };
#endif
    gtests += testor;

    return 0;
}

NMS_API u32 invoke(const StrView masks[], u32 count) {
    auto&   tests = gTests();

    List<const Testor*> failes;
    u32 total_count     = 0;
    u32 ok_count        = 0;
    u32 ignore_count    = 0;

    console::writeln("[===== nms.test ======]");

    for (auto& test: tests) {
        auto stat = test.match(masks, count);
        auto ret  = test.invoke(stat);

        if (!stat) {
            ++ignore_count;
        }
        else {
            ++total_count;

            if (ret) {
                ++ok_count;
            }
            else {
                failes += &test;
            }
        }
    }

    const auto fail_count = total_count - ok_count;
    console::writeln("[===== nms.test ======]");
    console::writeln("\033[1;32m[ok    ]\033[0m {}", ok_count);
    console::writeln("\033[1;33m[ignore]\033[0m {}", ignore_count);
    console::writeln("\033[1;31m[fail  ]\033[0m {}", fail_count);
    for(auto test: failes) {
        console::writeln("  |- {}", test->name);
    }
    console::writeln("[===== nms.test ======]");
    return fail_count;
}

}

NMS_ABI int test(int argc, const char* argv[]) {
    using namespace nms;

    StrView masks[256];
    for (auto i = 0; i < 256 && i < argc; ++i) {
        masks[i] = cstr(argv[i]);
    }
    auto ret = nms::test::invoke(masks, argc);
    return int(ret);
}

