#pragma once

#ifdef _WIN32

#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning(disable: 4127)      // conditional expression is constant
#pragma warning(disable: 4505)      // unreferenced inline function has been removed
#pragma warning(disable: 4514)      // unreferenced inline function has been removed
#pragma warning(disable: 4710)      // function not inlined
#pragma warning(disable: 4711)      // function performed inlined
#pragma warning(disable: 4820)      // n bytes padding add after data member ...
#pragma warning(disable: 6326)      // potential comparison of a constant with another
#endif

/* ucrt */
#define _CRT_DECLARE_NONSTDC_NAMES  1
#define _CRT_NONSTDC_NO_WARNINGS    1
#define _CRT_SECURE_NO_WARNINGS     1
#define _CRT_OBSOLETE_NO_DEPRECATE  1

/* stdc */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include "posix/threads.h"
#define STDOUT_FILENO   1
#define STDIN_FILENO    1
#define STDERR_FILENO   1

/* sys */
#include <sys/stat.h>
#undef  _stat

/* ucrt */
#include <corecrt_io.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#pragma warning(pop)

#ifdef __cplusplus
extern "C" {
#endif

/* === built in === */
inline unsigned __builtin_clz(unsigned __int32 x) {
    unsigned long r = 0;
    _BitScanReverse(&r, x);
    return 31 - r;
}

inline unsigned __builtin_clzll(unsigned __int64 x) {
    unsigned long r = 0;
    _BitScanReverse64(&r, x);
    return 63 - r;
}

/* === unistd === */
void _init_tty(int fd);

inline int ustd_isatty(int fd) {
    int ret = _isatty(fd);
    if (ret != 0) {
        static int has_init[3] = { 0, 0, 0 };
        if (fd < 3 && has_init[fd] == 0) {
            has_init[fd] = 1;
            _init_tty(fd);
        }
    }
    return ret;
}
#define isatty ustd_isatty

/* === time === */
typedef enum {
    CLOCK_MONOTONIC
} clockid_t;

int clock_gettime_monotonic(struct timespec* tp);

inline int clock_gettime(clockid_t clk_id, struct timespec* tp) {
    switch(clk_id) {
    case CLOCK_MONOTONIC:
        return clock_gettime_monotonic(tp);
    default:
        return -1;
    }
}

/* === iocto === */
enum {
    TIOCGWINSZ
};

struct winsize
{
    unsigned short int ws_row;
    unsigned short int ws_col;
};

int _ioctl_winsize(int fd, struct winsize* w);

inline int ioctl(int fd, unsigned long request, winsize* w) {
    switch(request) {
    case TIOCGWINSZ:
        return _ioctl_winsize(fd, w);
    default:
        return -1;
    }
}

#ifdef __cplusplus
}
#endif

#endif