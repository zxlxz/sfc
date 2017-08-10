#pragma once

#ifdef NMS_OS_UNIX

// stdc
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

// posix
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

// linux
#include <malloc.h>

// system
#include <sys/stat.h>

// c11-threads
using thrd_t    = pthread_t;
using mtx_t     = pthread_mutex_t;
using cnd_t     = pthread_cond_t;

#endif
