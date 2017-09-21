
#include <nms/core.h>
#include <nms/test.h>
#include <nms/io/console.h>
#include <nms/util/library.h>

using namespace nms;

static void show_help() {
    io::console::writeln("usage: ");
    io::console::writeln("    nms.test [@lib]... [+mask]... [-mask]... ");
}

int main(int argc, const char* argv[]) {
    List<Library, 32> libs;
    List<StrView, 32> masks;

    for (auto i = 1; i < argc; ++i) {
        auto str = StrView{ argv[i], nms::strlen(argv[i]) };
        if (str[0] == '@') {
            libs += str.slice(1, -1);
        }
        else if (str[0] == '+' || str[0] == '-') {
            masks += str;
        }
        else {
            show_help();
            return -1;
        }
    }

    auto ret = test::invoke(masks);
    return int(ret);
}
