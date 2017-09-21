#include <nms/test.h>
#include <nms/util/stackinfo.h>

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

    bool match(const View<StrView>& masks) const {
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

        const auto cond = cnt_pos>0 && cnt_neg ==0;
        return cond;
    }

    bool invoke(bool stat) const {
        const char fmt_run[]    = "\033[1;36m[>>] {:6.3} {}\033[0m";
        const char fmt_ok[]     = "\033[1;32m[<<] {:6.3} {}\033[0m";
        const char fmt_fail[]   = "\033[1;31m[<<] {:6.3} {}\033[0m";
        const char fmt_pass[]   = "\033[1;33m[--] {:6.3} {}\033[0m";
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
            auto& type_id = typeid(e);
            log::error("throw `{}`: {}", type_id, e);
            auto& stacks = IException::get_stackinfo();
            console::writeln("{}", stacks);
            console::writeln(fmt_fail, clock(), name);
            return false;
        }
        catch (...) {
            console::writeln(fmt_fail, clock(), name);
            return false;
        }
    }
};

static auto& gTests() {
    static List<Testor, 1024> tests;
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

    List<const Testor*, 256> failes;
    u32 total_count  = 0;
    u32 ok_count     = 0;
    u32 ignore_count = 0;

    const char logo[] = R"(
         _   _ ___  ___ _____ _____         _
        | \ | ||  \/  |/  ___|_   _|       | |
        |  \| || .  . |\ `--.  | | ___  ___| |_
        | . ` || |\/| | `--. \ | |/ _ \/ __| __|
        | |\  || |  | |/\__/ / | |  __/\__ \ |_
        \_| \_/\_|  |_/\____(_)\_/\___||___/\__|

)";
    console::writeln(logo);

    // show current time
    {
        const auto now = DateTime::now();
        char buff[128];
        auto len = snprintf(buff, sizeof(buff), "%04hu-%02hu-%02hu %02hu:%02hu:%02hu", now.year, now.month, now.day, now.hour, now.minute, now.second);
        auto msg = StrView{ buff, u32(len) };
        io::log::info("{}", msg);
    }

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
    console::writeln("\033[1;32m[==]\033[0m ok:     {}", ok_count);
    console::writeln("\033[1;33m[**]\033[0m ignore: {}", ignore_count);
    console::writeln("\033[1;31m[!!]\033[0m fail:   {}", fail_count);

    for (auto i = 0u; i < failes.count(); ++i) {
        auto& test = failes[i];

        if (i + 1 < failes.count()) {
            console::writeln("  \033(0tq\033(B{}", test->name);
        }
        else {
            console::writeln("  \033(0mq\033(B{}", test->name);
        }
    }

    console::writeln("[===== nms.test ======]");
    return fail_count;
}

}
