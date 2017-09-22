#include <nms/test.h>
#include <nms/util/stackinfo.h>

namespace nms
{

NMS_API StackInfo& IException::get_stackinfo() {
    static thread_local StackInfo stack_info;
    return stack_info;
}

NMS_API void IException::set_stackinfo() {
    auto&       global_info     = get_stackinfo();
    StackInfo   current_info(1);
    global_info = move(current_info);
}

NMS_API i32 ESystem::system_error() {
    return errno;
}

NMS_API void ESystem::format(IString& buf) const {
    U8String<512> message;

#if defined(NMS_OS_WINDOWS)
    strerror_s(message.data(), message.capacity(), eid_);
#elif defined(NMS_OS_APPLE)
    // XSI Tver
    const auto ret = strerror_r(eid_, message.data(), message.capacity());
    if (ret != 0) {
        return;
    }
#elif defined(NMS_OS_UNIX)
    // GNU Tver
    auto ptr = strerror_r(eid_, message.data(), message.capacity());
    if (ptr != message.data()) {
        ::free(ptr);
        return;
    }
#endif
    const auto message_len = strlen(message.data());
    message._resize(message_len);
    sformat(buf, "system error({}): {}", eid_, message);
}

}
