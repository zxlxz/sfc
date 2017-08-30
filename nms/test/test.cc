
#include <nms/test.h>
#include <nms/io.h>
#include <nms/util/stacktrace.h>

namespace nms
{
const CallStacks& gExceptionStacks();
}

namespace nms::test
{

using namespace nms::io;

struct Testor
{
    StrView name;
    void(*func)();

    bool match(const StrView& mask) const {
        if (mask.isEmpty()) {
            return true;
        }

        const auto mask_dat = mask.slice(1,  -1);
        const auto name_dat = name.slice(0u, mask_dat.count()-1);
        const auto stat = mask_dat == name_dat;
        return stat;
    }

    bool match(const View<StrView> masks) const {
        if (masks.count() == 0) {
            return true;
        }

        auto cnt_pos = 0;
        auto cnt_neg = 0;
        for(auto& mask: masks) {
            if (mask.isEmpty()) {
                continue;
            }
            auto c = mask[0];
            if (c == '+') {
                if (match(mask)) {
                    ++cnt_pos;
                }
            }
            if (c == '-') {
                if (match(mask)) {
                    ++cnt_neg;
                }
            }
        }

        return cnt_pos>=0 && cnt_neg ==0;
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
            return true;
        }
        catch (const IException& e) {
            log::error("throw nms::IException {}", e);
            {
                String str;
                auto& stacks = gExceptionStacks();

                const auto stacks_cnt = stacks.count();
                for (auto i = 0u; i < stacks_cnt; ++i) {
                    (i + 1 != stacks_cnt)
                        ? sformat(str, cstr("\t ├─{:2}: {}\n"), i, stacks[i])
                        : sformat(str, cstr("\t └─{:2}: {}"),   i, stacks[i]);
                }
                console::writeln(str);
            }

            console::writeln(fmt_fail, clock(), name);
            return false;
        }
        catch (...) {
            return false;
        }
    }
};

static auto& gTests() {
    static List<Testor> tests;
    return tests;
}

NMS_API u32 install(StrView name, void(*func)()) {
    static auto& gtests = gTests();

    // sizeof("struct ") = 7
    // sizeof("_tag")    = 4
#if defined(NMS_CC_MSVC)
    Testor testor{ name.slice(7, -5), func };
#elif defined(NMS_CC_CLANG)
    Testor testor{ name.slice(0, -5), func };
#elif defined(NMS_CC_GNUC)
    Testor testor{ name.slice(0, -5), func };
#else
    Testor testor{ name, func };
#endif
    gtests += testor;
    return 0;
}

NMS_API u32 invoke(const View<StrView>& masks) {
    auto&   tests = gTests();

    List<const Testor*> failes;
    u32 total_count     = 0;
    u32 ok_count        = 0;
    u32 ignore_count    = 0;

    const char logo[] = R"(
         _   _ ___  ___ _____ _____         _   
        | \ | ||  \/  |/  ___|_   _|       | |  
        |  \| || .  . |\ `--.  | | ___  ___| |_ 
        | . ` || |\/| | `--. \ | |/ _ \/ __| __|
        | |\  || |  | |/\__/ / | |  __/\__ \ |_ 
        \_| \_/\_|  |_/\____(_)\_/\___||___/\__|

)";
    printf("%s", logo);
    auto now = DateTime::now();
    printf("[ %04d-%02d-%02d %02d:%02d:%02d ]\n", now.year, now.month, now.day, now.hour, now.minute, now.second);

    for (auto& test: tests) {
        auto stat = test.match(masks);
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
        console::writeln("  ├─{}", test->name);
    }
    console::writeln("[===== nms.test ======]");
    return fail_count;
}

}

