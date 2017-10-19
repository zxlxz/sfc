#ifndef _USTD_CONFIG_H
#define _USTD_CONFIG_H

#include "ustd/detail/thread_types.h"
using thrd_t = ustd::thrd_t;
using mtx_t  = ustd::mtx_t;
using cnd_t  = ustd::cnd_t;

#define _THREADS_C
#if     defined(_WIN32)
#include "sys/win32.h"
#elif   defined(__linux)
#include "sys/linux.h"
#elif   defined(__APPLE__)
#include "sys/apple.h"
#endif

#undef stdin    // 0
#undef stdout   // 1
#undef stderr   // 2

#define _USTD_MODULE 0
#include "ustd.h"

#endif


