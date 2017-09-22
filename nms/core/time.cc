#include <nms/core.h>

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
    const time_t clock = stamp;

    struct tm tm = {};
#ifdef NMS_OS_WINDOWS
    ::gmtime_s(&tm, &clock);
#else
    ::gmtime_r(&clock, &tm);
#endif
    tm.tm_year += 1900;
    year    = u16(tm.tm_year);
    month   = u16(tm.tm_mon + 1);
    day     = u16(tm.tm_mday);
    hour    = u16(tm.tm_hour);
    minute  = u16(tm.tm_min);
    second  = u16(tm.tm_sec);
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

    tm.tm_year -= 1900;
#ifdef NMS_OS_WINDOWS
    const auto result = ::_mkgmtime(&tm);
#else
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
        u32(tm.tm_year) + 1900,
        u32(tm.tm_mon)+1,
        u32(tm.tm_mday),
        u32(tm.tm_hour),
        u32(tm.tm_min),
        u32(tm.tm_sec)
    };
}

NMS_API DateTime DateTime::parse(StrView str) {
    DateTime dt;
    auto cstr = str.data();
    (void)sscanf(cstr, "%hu-%hu-%huT%hu:%hu:%hu", &dt.year, &dt.month, &dt.day, &dt.hour, &dt.minute, &dt.second);
    return dt;
}

NMS_API void DateTime::format(IString& buf, StrView /*fmt*/) const {
    const auto len = buf.count();
    buf.reserve(len+32);
    const auto dat = buf.data();
    const auto ret = ::snprintf(dat+len, 32, "%hu-%02hu-%02huT%02hu:%02hu:%02hu", year, month, day, hour, minute, second);
    if (ret > 0) {
        buf._resize(len + u32(ret));
    }
}

}
