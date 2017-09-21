#pragma once

#ifdef NMS_BUILD

// compiler support
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <ctype.h>
#include <inttypes.h>
#include <float.h>
#include <limits>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <locale.h>

// microsoft C-Runtime Library
#ifdef NMS_OS_WINDOWS
#include <corecrt.h>
#include <corecrt_io.h>
#include <corecrt_malloc.h>
#include <corecrt_memory.h>
#include <corecrt_share.h>
#endif

// unix posix
#ifdef NMS_OS_UNIX
#include <pthread.h>
#include <semaphore.h>
#endif

// macos BSD library
#ifdef NMS_OS_APPLE
#include <malloc/malloc.h>
#include <mach-o/dyld.h>
#endif

// linux Glibc library
#ifdef NMS_OS_LINUX
#include <malloc.h>
#endif

#endif

/* === stdc === */
#include <stddef.h>
#include <stdint.h>
#include <math.h>

// c11 threads
#ifdef NMS_OS_WINDOWS
struct sem_t;

using thrd_t = struct thrd_st*;
using mtx_t  = struct mtx_st*;
using cnd_t  = struct cnd_st*;
#endif

#ifdef NMS_OS_APPLE
#ifdef NMS_BUILD
using thrd_t = pthread_t;
using mtx_t  = pthread_mutex_t;
using cnd_t  = pthread_cond_t;
#else

using thrd_t = struct pthread_st*;

struct mtx_t {
    long sig;
    char opaque[56];
};

struct cnd_t {
    long sig;
    char opaque[40];
};
#endif
#endif

#ifdef NMS_OS_LINUX
#include <pthread.h>
#include <semaphore.h>
using thrd_t = pthread_t;
using mtx_t  = pthread_mutex_t;
using cnd_t  = pthread_cond_t;
#endif
