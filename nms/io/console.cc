#include <nms/core.h>
#include <nms/io/console.h>
#include <nms/thread.h>

namespace nms::io::console
{

NMS_API void writes(const StrView text[], u32 n) {

#ifdef NMS_OS_WINDOWS
    static auto _static_init = [] {
        ::system("chcp 65001 > NUL");
        return 0;
    }();
    (void)_static_init;
#endif

    static thread::Mutex _mutex;
    thread::LockGuard lock(_mutex);
    for (u32 i = 0; i < n; ++i) {
        auto dat = text[i].data();
        auto len = text[i].count();
        fwrite(dat, 1, len, stdout);
    }
    fflush(stdout);
}

}

