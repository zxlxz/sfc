#include <windows.h>
#include <time.h>

int clock_gettime_monotonic(struct timespec* tp) {
    static LARGE_INTEGER freq;

    static int has_init = 0;
    if (has_init == 0) {
        QueryPerformanceFrequency(&freq);
        has_init = 1;
    }

    LARGE_INTEGER res;
    BOOL eid = QueryPerformanceCounter(&res);

    if (eid == FALSE) {
        return -1;
    }

    long long secs  = (res.QuadPart / freq.QuadPart);
    long long nanos = (res.QuadPart % freq.QuadPart) * 1000000000 / freq.QuadPart;

    tp->tv_sec  = (time_t)secs;
    tp->tv_nsec = (long)nanos;
    return 0;
}
