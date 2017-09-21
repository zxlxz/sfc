#include <nms/test.h>
#include <nms/util/stackinfo.h>
#include <nms/util/library.h>

#ifdef NMS_CC_MSVC
using namespace nms;

extern "C" {
    void*   GetCurrentProcess();
    ushort  RtlCaptureStackBackTrace(u32 FramesToSkip, u32 FramesToCapture, void** BackTrace, u32* BackTraceHash);

    int     SymInitialize(void* hProcess, const char* path, int fInvadeProcess);
    int     SymFromAddr(void* hProcess, u64 Address, u64* Displacement, struct _SYMBOL_INFO* Symbol);
}

struct _SYMBOL_INFO
{
    u32     SizeOfStruct;
    u32     TypeIndex;        // Type Index of symbol
    u64     Reserved[2];
    u32     Index;
    u32     Size;
    u64     pModBase;          // Base Address of module comtaining this symbol
    u32     Flags;
    u64     Value;            // Value of symbol, ValuePresent should be 1
    u64     Address;          // Address of symbol including base address of module
    u32     Register;         // register holding value or pointer to value
    u32     Scope;            // scope of the symbol
    u32     Tag;              // pdb classification
    u32     NameLen;          // Actual length of name
    u32     MaxNameLen;
    union {
        char    Name[1];      // Name of symbol
        char    dli_sname[1]; // Name of symbol
    };
};

using Dl_info = _SYMBOL_INFO;

static auto getCurrentProcess() {
    static auto proc = GetCurrentProcess();
    return proc;
}

static auto gDbgHelpLibrary(StrView name) {
    static Library dbghelp_lib("DbgHelp.dll");
    auto func = dbghelp_lib[name];
    return func;
}

#define NMS_DBGHELP_FUNC(name) static_cast<decltype(name)*>(gDbgHelpLibrary(#name))

static __forceinline auto backtrace(void** stacks, int count) {
    static auto proc        = getCurrentProcess();
    static auto sym_init    = NMS_DBGHELP_FUNC(SymInitialize);
    static auto init        = sym_init(proc, nullptr, true);  (void)init;

    auto ret = RtlCaptureStackBackTrace(0u, u32(count), stacks, nullptr);
    return ret;
}

static auto dladdr(void* handle, Dl_info* info) {
    static auto proc            = getCurrentProcess();
    info->MaxNameLen            = 512;
    info->SizeOfStruct          = sizeof(_SYMBOL_INFO);
    static auto pSymFromAddr    = NMS_DBGHELP_FUNC(SymFromAddr);
    auto ret = pSymFromAddr(proc, reinterpret_cast<u64>(handle), nullptr, info);

    return ret;
}
#endif

namespace nms
{

NMS_API void StackInfo::init() {
    auto cnt = nms::numel(stacks_);
    auto ret = backtrace(stacks_, i32(cnt));
    count_ = u32(ret);
}

NMS_API void StackInfo::Frame::format(IString& buff) const {
    if (ptr == nullptr) {
        buff += StrView("<null>");
        return;
    }

    struct {
        Dl_info info;
        char    buff[512];
    } info_ext;

    auto ret = dladdr(ptr, &info_ext.info);
    if (ret == 0) {
        buff += StrView("<unknow>");
        return;
    }
    auto name = StrView{ info_ext.info.dli_sname, u32(strlen(info_ext.info.dli_sname)) };

#ifdef NMS_CC_MSVC
    buff += name;
#else
    char out_name[4096];
    size_t length = sizeof(out_name);
    int status = 0;
    auto cxx_buff = abi::__cxa_demangle(name.data(), out_name, &length, &status);

    if (status == 0) {
        buff += StrView{ cxx_buff, strlen(cxx_buff) };

        if (cxx_buff != out_name) {
            ::free(cxx_buff);
        }
    }
    else if (cxx_buff != nullptr) {
        buff += StrView{ cxx_buff, strlen(cxx_buff) };
    }
    else {
        buff += StrView("<empty>");
    }
#endif
}

NMS_API void StackInfo::format(IString& buff) const {
    auto cnt = count();

    for (u32 i = 0; i < cnt; ++i) {
        auto stack = (*this)[i];
        i + 1 < cnt
            ? sformat(buff, "\t\033(0tq\033(B{:2}: {}\n", i, stack)
            : sformat(buff, "\t\033(0mq\033(B{:2}: {}\n", i, stack);
    }
}

#pragma region unittest
nms_test(stacktrace) {
    StackInfo stacks;
    io::log::info("stacks = \n{}", stacks);
}
#pragma endregion

}