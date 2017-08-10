#define module cumodule
#include <nms/cuda/nvrtc.h>
#undef module

#include <nms/core.h>
#include <nms/io.h>
#include <nms/test.h>
#include <nms/util/library.h>

#include <nms/cuda/runtime.h>
#include <nms/cuda/engine.h>
#include <nms/cuda/array.h>

namespace nms::cuda
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

NMS_API void Program::addSrc(StrView src) {
    src_ += src;
}

NMS_API void Program::addFile(const io::Path& path) {
    auto src = io::loadString(path);
    addSrc(src);
}

NMS_API u32 Program::add_foreach(StrView func, StrView ret_type, StrView arg_type) {
    if (src_.count() == 0) {
        return 0;
    }

    sformat(src_, "__kernel__ void nms_cuda_foreach_{}(\n", cnt_);
    sformat(src_, "    {} ret,\n", ret_type);
    sformat(src_, "    {} arg)\n", arg_type);
    src_ += "{\n";
    sformat(src_, "    nms::cuda::foreach<{}>(ret, arg);\n", func);
    src_ += "}\n\n";

    return cnt_++;
}

NMS_API bool Program::compile() {
    static const char*  argv[] = { "--std=c++11", "-default-device", "-restrict", "--use_fast_math", "-arch=compute_30"};
    static const int    argc = sizeof(argv) / sizeof(argv[0]);

    // create program
    nvrtcProgram nvrtc = nullptr;
    NMS_NVRTC_DO(nvrtcCreateProgram)(&nvrtc, src_.cstr(), nullptr, 0, nullptr, nullptr);
    if (nvrtc == nullptr) {
        io::log::error("nms.cuda.Program: cannot create program.");
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


NMS_API Program& gProgram() {
#include <nms/cuda/kernel.h>
    static auto _init = [] {
        const char path[] = "#/include/nms.cuda.kernel.h";

        try {
            if (!io::exists(path)) {
                io::TxtFile file(path, io::TxtFile::Write);
                file.write(kernel_src);
            }
        }
        catch (...) {
        }

        return 0;
    }();

    static Program program(kernel_src);
    return program;
}

}


#pragma region unittest

struct nms_cuda_Engine_test_kernel
{
    template<class ...Targ>
    auto operator()(Targ&& ...args) {
        return nms_cuda_Engine_test_kernel(nms::math::lambda_cast(args)...);
    }
};

static const char nms_cuda_Engine_test_kernel[] = R"(
__kernel__ void nms_cuda_Engine_test(nms::View<nms::f32,2> v) {
    const auto ix = blockIdx.x*blockDim.x+threadIdx.x;
    const auto iy = blockIdx.y*blockDim.y+threadIdx.y;
    v(ix, iy) = ix+iy*0.1f;
})";

namespace nms::cuda
{

nms_test(Engine) {
    gProgram().addSrc(nms_cuda_Engine_test_kernel);

    cuda::Array<f32, 2> dv({ 16, 16 });
    invoke<struct nms_cuda_Engine_test_kernel>(dv);

    host::Array<f32, 2> hv(dv.size());
    hv <<= dv;
    io::console::writeln("v = {|}", hv.slice({ 0u, 8u }, { 0u, 8u }));
}

}

#pragma endregion




