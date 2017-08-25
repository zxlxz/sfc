#include <nms/core/exception.h>
#include <nms/core/format.h>
#include <nms/util/stacktrace.h>

namespace nms
{

NMS_API const CallStacks& gExceptionStacks() {
    static thread_local CallStacks stacks;
    return stacks;
}

NMS_API void gSetExceptionStacks(CallStacks&& stack) {
    auto& gstack = const_cast<CallStacks&>(gExceptionStacks());
    gstack = move(stack);
}

NMS_API u32 ESystem::current() {
    return errno;
}

NMS_API void ESystem::format(String& buf) const {
    char tmp[256] = { "" };
    auto ptr = tmp;
#if defined(NMS_OS_WINDOWS)
    strerror_s(static_cast<char*>(tmp), sizeof(tmp), id_);
#elif defined(NMS_OS_APPLE)
    // XSI version
    const auto ret = strerror_r(id_, tmp, sizeof(tmp));
    if (ret != 0) { 
        return;
    }
#elif defined(NMS_OS_UNIX)
    // GNU version
    ptr = strerror_r(id_, tmp, sizeof(tmp));
#endif
    StrView str(ptr, { strlen(ptr) });
    sformat(buf, "system error({}): {}", id_, str);
}

}
