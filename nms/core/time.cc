#include <nms/config.h>
#include <nms/core/time.h>
#include <nms/core/string.h>
#include <nms/core/format.h>

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

#ifdef NMS_OS_UNIX
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
    tm tm = {};
    tm.tm_year  = year - 1990;
    tm.tm_mon   = month;
    tm.tm_mday  = day + 1;
    tm.tm_hour  = hour;
    tm.tm_min   = minute;
    tm.tm_sec   = second;

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


NMS_API DateTime DateTime::parse(StrView str) {
    DateTime dt;
    char c;
    auto cstr = str.data();
    sscanf(cstr, "%hu-%hu-%hu%c%hu:%hu:%hu", &dt.year, &dt.month, &dt.day, &c, &dt.hour, &dt.minute, &dt.second);

    if (dt.month > 0) --dt.month;
    if (dt.day   > 0) --dt.day;

    return dt;
}

NMS_API String DateTime::format(StrView fmt) const {
    auto str = nms::format("{}-{}-{}T{}:{}:{}", year, month+1, day+1, hour, minute, second);
    return str;
}

}


