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

// UTC
NMS_API void DateTime::init_stamp(i64 stamp) {
    struct tm tm = {};
#ifdef NMS_OS_WINDOWS
    ::gmtime_s(&tm, &stamp);
    tm.tm_year += 1900;
#else
    ::gmtime_r(&stamp, &tm);
    tm.tm_year += 1970;
#endif
    year    = tm.tm_year;
    month   = tm.tm_mon + 1;
    day     = tm.tm_mday;
    hour    = tm.tm_hour;
    minute  = tm.tm_min;
    second  = tm.tm_sec;
}

// UTC
NMS_API i64 DateTime::stamp() const {
    tm tm = {};
    tm.tm_year  = year;
    tm.tm_mon   = month- 1;
    tm.tm_mday  = day;
    tm.tm_hour  = hour;
    tm.tm_min   = minute;
    tm.tm_sec   = second;

#ifdef NMS_OS_WINDOWS
    // since 1900
    tm.tm_year -= 1900;
    const auto result = ::_mkgmtime(&tm);
#else
    tm.tm_year -= 1970;
    const auto result = ::timegm(&tm);
#endif
    return result;
}

NMS_API DateTime DateTime::now() {
    const auto  stamp = ::time(nullptr);
    struct tm   tm;
#ifdef NMS_OS_WINDOWS
    ::localtime_s(&tm, &stamp);
#else
    ::localtime_r(&stamp, &tm);
#endif
    return { 
        u32(tm.tm_year)+1990,
        u32(tm.tm_mon)+1,
        u32(tm.tm_mday),
        u32(tm.tm_hour),
        u32(tm.tm_min),
        u32(tm.tm_sec)
    };
}


NMS_API DateTime DateTime::parse(StrView str) {
    DateTime dt;
    char c;
    auto cstr = str.data();
    sscanf(cstr, "%hu-%hu-%hu%c%hu:%hu:%hu", &dt.year, &dt.month, &dt.day, &c, &dt.hour, &dt.minute, &dt.second);
    return dt;
}

NMS_API void DateTime::format(String& buff, StrView fmt) const {
    nms::sformat(buff, "{}-{}-{}T{}:{}:{}", year, month, day, hour, minute, second);
}

}


