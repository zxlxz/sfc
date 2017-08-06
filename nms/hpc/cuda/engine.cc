#define module cumodule
#include <nms/hpc/cuda/nvrtc.h>
#undef module

#include <nms/core.h>
#include <nms/io.h>
#include <nms/util/library.h>

#include <nms/hpc/cuda/runtime.h>
#include <nms/hpc/cuda/engine.h>

namespace nms::hpc::cuda
{

#pragma region library

#define NMS_NVRTC_DEF(F)                                                            \
    F(nvrtcCreateProgram),      F(nvrtcDestroyProgram),     F(nvrtcCompileProgram), \
    F(nvrtcAddNameExpression),  F(nvrtcGetLoweredName),                             \
    F(nvrtcGetProgramLog),      F(nvrtcGetProgramLogSize),                          \
    F(nvrtcGetPTX),             F(nvrtcGetPTXSize)
    
enum NVRTCTable
{
#define NMS_NVRTC_IDX(name) $##name
        NMS_NVRTC_DEF(NMS_NVRTC_IDX)
#undef  NMS_NVRTC_IDX
};


static Library::Function nvrtcFun(u32 id) {
    static Library lib("nvrtc64_80.dll");

    static Library::Function funcs[] = {
#define NMS_NVRTC_FUN(name) lib[cstr(#name)]
        NMS_NVRTC_DEF(NMS_NVRTC_FUN)
#undef  NMS_NVRTC_FUN
    };

    auto ret = funcs[id];
    return ret;
}

#define NMS_NVRTC_DO(name)   static_cast<decltype(name)*>(nvrtcFun($##name))

#pragma endregion

#pragma region program

void driver_init();

NMS_API Program::Program(StrView src)
    : src_{ src }
    , ptx_{}
    , cnt_{ 0 }
{}

NMS_API Program::~Program() 
{}

NMS_API u32 Program::add_foreach(StrView func, StrView ret_type, StrView arg_type) {
    if (src_.count() == 0) {
        return 0;
    }

    sformat(src_, "__global__ void nms_hpc_cuda_foreach_{}(\n", cnt_);
    sformat(src_, "    {} ret,\n", ret_type);
    sformat(src_, "    {} arg)\n", arg_type);
    src_ += "{\n";

    src_ += "}\n";

    return cnt_++;
}

NMS_API bool Program::compile() {
    static const char*  argv[] = { "--std=c++11", "-default-device", "-restrict", "--use_fast_math", "-arch=compute_30"};
    static const int    argc = sizeof(argv) / sizeof(argv[0]);

    // create program
    nvrtcProgram nvrtc = nullptr;
    NMS_NVRTC_DO(nvrtcCreateProgram)(&nvrtc, src_.cstr(), nullptr, 0, nullptr, nullptr);
    if (nvrtc == nullptr) {
        io::log::error("nms.hpc.cuda.Program: cannot create program.");
    }

    // compile
    auto ret = NMS_NVRTC_DO(nvrtcCompileProgram)(nvrtc, argc, argv);

    if (int(ret) != 0) {
        io::log::error("cuda compile failed.");

        size_t nvrtc_log_size = 0;
        NMS_NVRTC_DO(nvrtcGetProgramLogSize)(nvrtc, &nvrtc_log_size);

        String nvrtc_log;
        nvrtc_log.resize(u32(nvrtc_log_size));
        NMS_NVRTC_DO(nvrtcGetProgramLog)(nvrtc, nvrtc_log.data());
        io::console::writeln(nvrtc_log);

        return false;
    }

    // get ptx
    size_t ptx_len = 0;
    NMS_NVRTC_DO(nvrtcGetPTXSize)(nvrtc, &ptx_len);

    ptx_.reserve(u32(ptx_len));
    ptx_.resize(u32(ptx_len)-1);
    NMS_NVRTC_DO(nvrtcGetPTX)(nvrtc, ptx_.data());

    // destroy program
    NMS_NVRTC_DO(nvrtcDestroyProgram)(&nvrtc);

    return true;
}


#pragma endregion

#pragma region executor
NMS_API Module& ForeachExecutor::gModule() {
    static StrView ptx_src = [=] {
        const io::Path src_path("#/config/nms.hpc.cuda.program.cu");
        const io::Path ptx_path("#/config/nms.hpc.cuda.program.ptx");
        auto& program = gProgram();

        // if not exists, try save
        if (!io::exists(ptx_path)) {
            program.compile();

            io::TxtFile src_file(src_path, io::TxtFile::Write);
            src_file.write(program.src());

            io::TxtFile ptx_file(ptx_path, io::TxtFile::Write);
            ptx_file.write(program.ptx());
        }

        static auto ptx = io::loadString(ptx_path);
        return StrView(ptx);
    }();

    static Module value(ptx_src);
    return value;
}


NMS_API Program& ForeachExecutor::gProgram() {
#include <nms/hpc/cuda/kernel.h>
    static Program program(kernel_src);
    return program;
}


#pragma endregion


}
