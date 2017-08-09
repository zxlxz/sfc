#pragma once

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
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <dlfcn.h>

// system
#include <sys/stat.h>
#include <sys/mman.h>

// macos
#include <malloc/malloc.h>
#include <mach-o/dyld.h>
#include <execinfo.h>


// c11-threads
using thrd_t    = pthread_t;
using mtx_t     = pthread_mutex_t;
using cnd_t     = pthread_cond_t;
