#pragma once

#include <nms/core/base.h>
#include <nms/core/string.h>

namespace nms
{

/* wall time */
struct DateTime
{
    u16 year    = 0;    // year         [0~65535]
    u16 month   = 0;    // month        [1~12]
    u16 day     = 0;    // day of month [1~31]

    u16 hour    = 0;    // hour         [0~23]
    u16 minute  = 0;    // minute       [0~59]
    u16 second  = 0;    // second       [0~59]

    constexpr DateTime()
        : year(1990), month(1), day(1)
    {}

    explicit DateTime(i64 stamp) {
        init_stamp(stamp);
    }

    constexpr DateTime(u32 year, u32 month, u32 day)
        : year(u16(year)), month(u16(month)), day(u16(day))
    {}

    constexpr DateTime(u32 year, u32 month, u32 day, u32 hour, u32 minute, u32 second)
        : year(u16(year)), month(u16(month)), day(u16(day))
        , hour(u16(hour)), minute(u16(minute)), second(u16(second))
    {}

    NMS_API static DateTime parse(StrView str);
    NMS_API        void     format(IString& buff, StrView fmt) const;

#ifdef _M_CEE
    __forceinline DateTime(System::DateTime v)
        : year(v.Year), month(v.Month), day(v.Day)
        , hour(v.Hour), minute(v.Minute), second(v.Second)
    {}
#endif

    NMS_API i64 stamp() const;
    NMS_API static DateTime now();

    friend i64 operator-(const DateTime& a, const DateTime& b) {
        const auto ta = a.stamp();
        const auto tb = b.stamp();
        return ta - tb;
    }

private:
    NMS_API void init_stamp(i64 stamp);
};

/*!
 * process clock
 */
NMS_API f64 clock();

}
