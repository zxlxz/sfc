#include <nms/test.h>
#include <nms/util/stacktrace.h>
#include <nms/util/library.h>
#include <nms/io/console.h>

#ifdef NMS_CC_MSVC
using namespace nms;

extern "C" {
    void*   GetCurrentProcess();
    ushort  RtlCaptureStackBackTrace(ulong FramesToSkip, ulong FramesToCapture, void** BackTrace, ulong* BackTraceHash);

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

auto backtrace(void** stacks, int count) {
    static auto proc        = getCurrentProcess();
    static auto sym_init    = NMS_DBGHELP_FUNC(SymInitialize);
    static auto init        = sym_init(proc, nullptr, true);  (void)init;

    auto ret = RtlCaptureStackBackTrace(0, count, stacks, nullptr);
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

NMS_API void CallStacks::init() {
    auto ret = backtrace(stacks_, nms::count(stacks_));
    count_ = u32(ret);
}

NMS_API void CallStacks::Stack::format(String& buf) const {
    if (ptr == nullptr) {
        buf += cstr("<null>");
        return;
    }

    struct {
        Dl_info info;
        char    buff[512];
    } info_ext;

    auto ret = dladdr(ptr, &info_ext.info);
    if (ret == 0) {
        buf += cstr("<unknow>");
        return;
    }
    auto name = cstr(info_ext.info.dli_sname);

#ifdef NMS_CC_MSVC
    buf += name;
#else
    size_t length = 0;
    int status = 0;
    auto cxx_buff = abi::__cxa_demangle(name.data(), nullptr, &length, &status);
    if (status == 0) {
        buf += cstr(cxx_buff);
        ::free(cxx_buff);
    }
    else if (cxx_buff != nullptr) {
        buf += cstr(cxx_buff);
    }
    else {
        buf += cstr("<empty>");
    }
#endif
}

#pragma region unittest
nms_test(stacktrace) {
    CallStacks stacks;
    auto cnt = min(64u, stacks.count());

    String buff = "nms.Stack.backtrace:";
    for (u32 i = 0; i < cnt; ++i) {
        auto stack = stacks[i];
        i + 1 < cnt
            ? sformat(buff, "\n  ├─{:2}: {}", i, stack)
            : sformat(buff, "\n  └─{:2}: {}", i, stack);
    }
    io::log::info(buff);
}
#pragma endregion

}