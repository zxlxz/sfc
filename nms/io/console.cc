#include <nms/core.h>
#include <nms/io/console.h>
#include <nms/thread.h>

extern "C"
{
    void* GetStdHandle(long fildno);
    int SetConsoleMode(void*, unsigned long);
}

namespace nms::io::console
{

static auto _init_console() {
#ifdef NMS_OS_WINDOWS
    auto hout = ::GetStdHandle(-11);
    ::SetConsoleMode(hout, 0xF);
#endif
    return 0;
}

NMS_API void writes(const StrView text[], u32 n) {
    static auto init = _init_console();

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

