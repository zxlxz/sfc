
#include <nms/config.h>
#include <nms/core.h>

namespace nms::system
{

NMS_API String getenv(StrView name) {
    // check if name is empty
    if (name.count()==0) {
        return {};
    }

    auto cname = name.data();

    // check if name is cstring(null terminal)
    if (name[name.count()] != '\0') {
        char buff[512];
        if (name.count() >= u32(sizeof(buff)-1) ) {
            return "";
        }
        mcpy(buff, name.data(), name.count());
        buff[name.count()] = '\0';
        cname = buff;
    }

#ifdef NMS_OS_WINDOWS
    char buf[64] = { 0 };
    size_t size = 0;
    ::getenv_s(&size, buf, cname);
    if (size > 0) {
        size = size - 1;
    }
#else
    auto buf    = ::getenv(cname);
    auto size   = ::strlen(buf);
#endif
    return { buf, static_cast<u32>(size) };
}

NMS_API void sleep(double duration) {
#ifdef NMS_OS_WINDOWS
    _sleep(u32(duration * 1000));
#else
    struct timespec ts;
    u64 nsec    = u64(duration*1e9);

    ts.tv_sec   = nsec / 1000000000;
    ts.tv_nsec  = nsec % 1000000000;

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
    return;
#endif

}

NMS_API void beep(u32 freq, f64 duration) {
#ifdef NMS_OS_WINDOWS
    _beep(freq, u32(duration*1e3));
#else
    (void)freq;
    (void)duration;
    //::beep();
#endif
}

NMS_API void pause() {
#ifdef NMS_OS_WINDOWS
    ::system("pause");
#else
    //(void)::getch();
#endif
}

}
