#include <nms/test.h>
#include <nms/util/library.h>

#include <nms/cuda/runtime.h>
#include <nms/cuda/engine.h>
#include <nms/cuda/array.h>

extern "C"
{
    typedef struct _nvrtcProgram *nvrtcProgram;

    enum nvrtcResult
    { };

    const char *nvrtcGetErrorString(nvrtcResult result);

    nvrtcResult nvrtcCreateProgram(nvrtcProgram *prog, const char *src, const char *name, int numHeaders, const char * const *headers, const char * const *includeNames);
    nvrtcResult nvrtcDestroyProgram(nvrtcProgram *prog);

    nvrtcResult nvrtcCompileProgram(nvrtcProgram prog, int numOptions, const char * const *options);

    nvrtcResult nvrtcGetProgramLogSize(nvrtcProgram prog, size_t *logSizeRet);
    nvrtcResult nvrtcGetProgramLog(nvrtcProgram prog, char *log);

    nvrtcResult nvrtcGetPTXSize(nvrtcProgram prog, size_t *ptxSizeRet);
    nvrtcResult nvrtcGetPTX(nvrtcProgram prog, char *ptx);
}

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
#define NMS_NVRTC_FUN(name) lib[StrView{#name}]
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

NMS_API bool Program::compile() {
    static const char*  argv[] = { "--std=c++11", "-default-device", "-restrict", "--use_fast_math", "-arch=compute_30"};
    static const int    argc = sizeof(argv) / sizeof(argv[0]);

    // create program
    src_.reserve(src_.count() + 1);
    src_[src_.count()] = '\0';

    nvrtcProgram nvrtc = nullptr;
    NMS_NVRTC_DO(nvrtcCreateProgram)(&nvrtc, src_.data(), nullptr, 0, nullptr, nullptr);
    if (nvrtc == nullptr) {
        io::log::error("nms.cuda.Program: cannot create program.");
    }

    // compile
    auto ret = NMS_NVRTC_DO(nvrtcCompileProgram)(nvrtc, argc, argv);

    if (int(ret) != 0) {
        size_t nvrtc_log_size = 0;
        NMS_NVRTC_DO(nvrtcGetProgramLogSize)(nvrtc, &nvrtc_log_size);

        U8String<> nvrtc_log(static_cast<u32>(nvrtc_log_size));
        NMS_NVRTC_DO(nvrtcGetProgramLog)(nvrtc, nvrtc_log.data());
        io::console::writeln(nvrtc_log);

        return false;
    }

    // get ptx
    size_t ptx_len = 0;
    NMS_NVRTC_DO(nvrtcGetPTXSize)(nvrtc, &ptx_len);

    ptx_.reserve(u32(ptx_len));
    NMS_NVRTC_DO(nvrtcGetPTX)(nvrtc, ptx_.data());
    ptx_._resize(u32(ptx_len) - 1);

    // destroy program
    NMS_NVRTC_DO(nvrtcDestroyProgram)(&nvrtc);

    return true;
}
#pragma endregion

}
