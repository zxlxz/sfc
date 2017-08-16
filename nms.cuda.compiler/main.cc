
#include <nms/core.h>
#include <nms/cuda.h>
#include <nms/io.h>
#include <nms/cuda/kernel.h>

using namespace nms;

static void show_help() {
    io::console::writeln("usage: ");
    io::console::writeln("    nms.compiler [src.cu]... -o [dst.ptx]");
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        show_help();
        return 0;
    }

    StrView ptx_path("nms.cuda.program.ptx");
    String  srcs(nms_cuda_kernel_src);
    for (auto i = 1; i < argc; ++i) {
        auto path = cstr(argv[i]);
        if (path == "-o" && i + 1 < argc) {
            ptx_path = cstr(argv[i + 1]);
            i += 2;
            continue;
        }

        if (io::exists(path)) {
            sformat(srcs, "#line 1 \"{}\"\n", path);
            const auto src = io::loadString(path);
            srcs += StrView{ src };
        }
        else {
            io::console::writeln("cannot find {}.", path);
            return 0;
        }
    }
    cuda::Program program(srcs);

    auto stat = program.compile();
    if (!stat) {
        return 0;
    }

    auto ptx = program.ptx();
    io::TxtFile ptx_file(ptx_path, io::TxtFile::Write);
    ptx_file.write(ptx);

    return 0;
}
