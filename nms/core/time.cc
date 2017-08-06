#include <nms/config.h>
#include <nms/core/time.h>

namespace nms
{

#ifdef NMS_OS_WINDOWS
extern "C" {
    int QueryPerformanceFrequency(u64* freq);
    int QueryPerformanceCounter(u64* counter);
}

NMS_API f64 system_clock() {
    static const auto freq = [] {
        u64 val;
        QueryPerformanceFrequency(&val);
        return val;
    }();

    u64 clock;
    QueryPerformanceCounter(&clock);

    return static_cast<f64>(clock) / static_cast<f64>(freq);
}

static const f64 g_clock_start = system_clock();

NMS_API f64 clock() {
    const auto clock_now = system_clock();
    return clock_now - g_clock_start;
}
#endif

#ifdef NMS_OS_POSIX
NMS_API f64 system_clock() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    auto sec = spec.tv_sec;
    auto nsec = spec.tv_nsec;
    return f64(sec) + f64(nsec*1e-9);
};

static const auto g_clock_start = system_clock();

NMS_API f64 clock() {
    auto clock_now = system_clock();
    return clock_now - g_clock_start;
}
#endif

NMS_API i64 DateTime::stamp() const {
    tm            tm     = { year, month, day, hour, minute, second };
    const time_t  result = mktime(&tm);
    return result;
}

NMS_API DateTime DateTime::now() {
    const auto  stamp = time(nullptr);
    struct tm   tm;
#ifdef NMS_OS_WINDOWS
    localtime_s(&tm, &stamp);
#else
    localtime_r(&stamp, &tm);
#endif
    return { u32(tm.tm_year), u32(tm.tm_mon), u32(tm.tm_mday), u32(tm.tm_hour), u32(tm.tm_min), u32(tm.tm_sec) };
}

}


