#include <nms/core/exception.h>
#include <nms/core/format.h>
#include <nms/util/stacktrace.h>

namespace nms
{

NMS_API const ProcStacks& gExceptionStacks() {
    static thread_local ProcStacks stacks;
    return stacks;
}

NMS_API void IException::_init_stack_trace()
{
    auto& global_stacks  = const_cast<ProcStacks&>(gExceptionStacks());
    auto  current_stacks = ProcStacks();
    global_stacks = current_stacks;
}

NMS_API u32 ESystem::current() {
    return errno;
}

NMS_API void ESystem::format(String& buf) const {
    char tmp[256] = { "" };
    auto ptr = tmp;
#ifdef NMS_OS_WINDOWS
    strerror_s(static_cast<char*>(tmp), sizeof(tmp), id_);
#else
    ptr = strerror_r(id_, tmp, sizeof(tmp));
#endif
    StrView str(ptr, { strlen(ptr) });
    sformat(buf, "system error({}): {}", id_, str);
}

}
